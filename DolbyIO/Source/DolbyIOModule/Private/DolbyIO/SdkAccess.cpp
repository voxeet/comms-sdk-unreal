// Copyright 2022 Dolby Laboratories

#include "DolbyIO/SdkAccess.h"

#include "DolbyIO/DeviceManagement.h"
#include "DolbyIO/ErrorHandler.h"
#include "DolbyIO/Logging.h"
#include "DolbyIO/SdkEventObserver.h"

#include <dolbyio/comms/async_result.h>
#include <dolbyio/comms/sdk.h>

#include "Math/Rotator.h"

static std::string ToStdString(const FString& String)
{
	return TCHAR_TO_UTF8(*String);
}

namespace DolbyIO
{
	using namespace dolbyio::comms;

	FSdkAccess::FSdkAccess(ISdkEventObserver& Observer)
	    : Observer(Observer), ConferenceStatus(EConferenceStatus::destroyed)
	{
#if PLATFORM_WINDOWS
		static auto AlignedNew =
		    +[](std::size_t Count, std::size_t Al) { return operator new(Count, static_cast<std::align_val_t>(Al)); };
		static auto AlignedDelete =
		    +[](void* Ptr, std::size_t Al) { operator delete(Ptr, static_cast<std::align_val_t>(Al)); };
		sdk::set_app_allocator({operator new, AlignedNew, operator delete, AlignedDelete});
#endif

		sdk::log_settings LogSettings;
		LogSettings.sdk_log_level = log_level::INFO;
		LogSettings.media_log_level = log_level::OFF;
		LogSettings.log_directory = "";
		sdk::set_log_settings(LogSettings);
	}

	FSdkAccess::~FSdkAccess()
	{
		if (IsConnected())
		{
			try
			{
				wait(Sdk->conference().leave().then([this]() { return Sdk->session().close(); }));
			}
			catch (...)
			{
				MakeHandler(__LINE__).RethrowAndUpdateStatus();
			}
		}
	}

	void FSdkAccess::SetToken(const FToken& Token)
	try
	{
		if (!RefreshTokenCb)
		{
			Initialize(Token);
		}
		else
		{
			(*RefreshTokenCb)(ToStdString(Token));
			RefreshTokenCb.Reset(); // RefreshToken callback can be called only once
		}
	}
	catch (...)
	{
		MakeHandler(__LINE__).RethrowAndUpdateStatus();
	}

	void FSdkAccess::Initialize(const FToken& Token)
	try
	{
		LocalParticipantID.Reset();
		ParticipantIDs.Empty();
		Sdk.Reset(sdk::create(ToStdString(Token),
		                      [this](auto&& cb)
		                      {
			                      UE_LOG(LogDolbyIO, Log, TEXT("Refresh token requested"));
			                      RefreshTokenCb.Reset(cb.release());
			                      Observer.OnTokenNeededEvent();
		                      })
		              .release());
		Devices.Reset(MakeUnique<FDeviceManagement>(Sdk->device_management(), Observer,
		                                            [this](int Id) { return MakeHandler(Id); })
		                  .Release());

		Sdk->conference()
		    .add_event_handler([this](const conference_status_updated& Event) { UpdateStatus(Event.status); })
		    .on_error(MakeHandler(__LINE__));

		Sdk->conference()
		    .add_event_handler(
		        [this](const participant_added& Event)
		        {
			        ParticipantIDs.Add(Event.participant.user_id.c_str());
			        Observer.OnListOfRemoteParticipantsChangedEvent(ParticipantIDs);
		        })
		    .on_error(MakeHandler(__LINE__));

		Sdk->conference()
		    .add_event_handler(
		        [this](const participant_updated& Event)
		        {
			        const auto& ParticipantStatus = Event.participant.status;
			        if (ParticipantStatus)
			        {
				        if (*ParticipantStatus == participant_status::left)
				        {
					        ParticipantIDs.Remove(Event.participant.user_id.c_str());
					        Observer.OnListOfRemoteParticipantsChangedEvent(ParticipantIDs);
				        }
			        }
		        })
		    .on_error(MakeHandler(__LINE__));

		Sdk->conference()
		    .add_event_handler(
		        [this](const active_speaker_change& Event)
		        {
			        FParticipants ActiveSpeakers;
			        for (const auto& Speaker : Event.active_speakers)
			        {
				        ActiveSpeakers.Add(Speaker.c_str());
			        }
			        Observer.OnListOfActiveSpeakersChangedEvent(ActiveSpeakers);
		        })
		    .on_error(MakeHandler(__LINE__));

		Observer.OnInitializedEvent();
	}
	catch (...)
	{
		MakeHandler(__LINE__).RethrowAndUpdateStatus();
	}

	bool FSdkAccess::IsConnected() const
	{
		return ConferenceStatus == EConferenceStatus::joined;
	}

	namespace
	{
		using EConferenceStatus = FSdkAccess::EConferenceStatus;

		FString ToString(EConferenceStatus Status)
		{
			switch (Status)
			{
				case EConferenceStatus::creating:
					return "creating";
				case EConferenceStatus::created:
					return "created";
				case EConferenceStatus::joining:
					return "joining";
				case EConferenceStatus::joined:
					return "joined";
				case EConferenceStatus::leaving:
					return "leaving";
				case EConferenceStatus::left:
					return "left";
				case EConferenceStatus::destroyed:
					return "destroyed";
				case EConferenceStatus::error:
					return "error";
				default:
					return "unknown";
			};
		}
	}

	void FSdkAccess::UpdateStatus(EConferenceStatus Status)
	{
		ConferenceStatus = Status;
		UE_LOG(LogDolbyIO, Log, TEXT("Conference status: %s"), *ToString(ConferenceStatus));

		switch (ConferenceStatus)
		{
			case EConferenceStatus::joined:
				Observer.OnConnectedEvent();
				break;
			case EConferenceStatus::left:
			case EConferenceStatus::destroyed:
			case EConferenceStatus::error:
				Observer.OnDisconnectedEvent();
				break;
		}
	}

	void FSdkAccess::Connect(const FConferenceName& Conf, const FUserName& User)
	try
	{
		if (!Sdk)
		{
			UE_LOG(LogDolbyIO, Warning, TEXT("Cannot connect - not initialized"));
			return;
		}
		if (IsConnected())
		{
			UE_LOG(LogDolbyIO, Warning, TEXT("Cannot connect - already connected, please disconnect first"));
			return;
		}
		if (Conf.IsEmpty())
		{
			UE_LOG(LogDolbyIO, Warning, TEXT("Cannot connect - conference name cannot be empty"));
			return;
		}

		bIsDemo = Conf == "demo";

		using namespace dolbyio::comms::services;
		services::session::user_info UserInfo{};
		UserInfo.name = ToStdString(User);

		Sdk->session()
		    .open(MoveTemp(UserInfo))
		    .then(
		        [this, Conf](auto&& User)
		        {
			        if (User.participant_id)
			        {
				        LocalParticipantID = User.participant_id->c_str();
				        Observer.OnLocalParticipantChangedEvent(LocalParticipantID);
			        }

			        if (bIsDemo)
			        {
				        return Sdk->conference().demo();
			        }

			        conference::conference_options Options{};
			        Options.alias = ToStdString(Conf);
			        Options.params.spatial_audio_style = spatial_audio_style::shared;

			        return Sdk->conference().create(Options);
		        })
		    .then(
		        [this](auto&& ConferenceInfo)
		        {
			        if (bIsDemo)
			        {
				        return async_result<conference_info>{MoveTemp(ConferenceInfo)};
			        }

			        conference::join_options Options{};
			        Options.constraints.audio = true;
			        Options.connection.spatial_audio = true;
			        return Sdk->conference().join(ConferenceInfo, Options);
		        })
		    .on_error(MakeHandler(__LINE__));
	}
	catch (...)
	{
		MakeHandler(__LINE__).RethrowAndUpdateStatus();
	}

	void FSdkAccess::Disconnect()
	{
		if (!IsConnected())
		{
			UE_LOG(LogDolbyIO, Warning, TEXT("Must be connected to disconnect"));
			return;
		}

		Sdk->conference()
		    .leave()
		    .then(
		        [this]()
		        {
			        LocalParticipantID.Reset();
			        Observer.OnLocalParticipantChangedEvent(LocalParticipantID);
			        ParticipantIDs.Empty();
			        Observer.OnListOfRemoteParticipantsChangedEvent(ParticipantIDs);
			        return Sdk->session().close();
		        })
		    .on_error(MakeHandler(__LINE__));
	}

	void FSdkAccess::UpdateViewPoint(const FVector& Position, const FRotator& Rotation)
	{
		if (!IsConnected())
		{
			return;
		}

		spatial_audio_batch_update Update;

		if (bIsDemo)
		{
			static float Angle = 0;
			Angle += 0.01f;
			for (const auto& Participant : ParticipantIDs)
			{
				Update.set_spatial_position(ToStdString(Participant),
				                            Participant[0] == '1'
				                                ? spatial_position{FMath::Cos(Angle), 0, FMath::Sin(Angle)}
				                            : Participant[0] == '2' ? spatial_position{-1, 0, 0}
				                                                    : spatial_position{1, 0, 0});
			}
		}

		// The default SDK spatial settings expect meters as unit of length,
		// Unreal uses centimeters for scale, so we divide every length by 100.
		// The default SDK coordinate system expects position arguments to mean (in order) right, up and
		// forward. In Unreal, right axis is +Y, up is +Z and forward is +X.
		const auto ScaledPosition = Position / 100;
		Update.set_spatial_position(ToStdString(LocalParticipantID),
		                            {ScaledPosition.Y, ScaledPosition.Z, ScaledPosition.X});
		Update.set_spatial_direction({Rotation.Pitch, Rotation.Yaw, Rotation.Roll});
		Sdk->conference().update_spatial_audio_configuration(MoveTemp(Update)).on_error(MakeHandler(__LINE__));
	}

	void FSdkAccess::MuteInput()
	{
		if (!IsConnected())
		{
			UE_LOG(LogDolbyIO, Warning, TEXT("Must be connected to mute input"));
			return;
		}

		Sdk->conference().mute(true).on_error(MakeHandler(__LINE__));
	}

	void FSdkAccess::UnmuteInput()
	{
		if (!IsConnected())
		{
			return;
		}

		Sdk->conference().mute(false).on_error(MakeHandler(__LINE__));
	}

	void FSdkAccess::MuteOutput()
	{
		if (!IsConnected())
		{
			UE_LOG(LogDolbyIO, Warning, TEXT("Must be connected to mute output"));
			return;
		}

		Sdk->conference().mute_output(true).on_error(MakeHandler(__LINE__));
	}

	void FSdkAccess::UnmuteOutput()
	{
		if (!IsConnected())
		{
			return;
		}

		Sdk->conference().mute_output(false).on_error(MakeHandler(__LINE__));
	}

	void FSdkAccess::GetAudioLevels()
	{
		if (!IsConnected())
		{
			UE_LOG(LogDolbyIO, Warning, TEXT("Must be connected to get audio levels"));
			return;
		}

		Sdk->conference()
		    .get_all_audio_levels()
		    .then(
		        [this](auto&& Levels)
		        {
			        FAudioLevels AudioLevels;
			        for (const audio_level& Level : Levels)
			        {
				        AudioLevels.Emplace(Level.participant_id.c_str(), Level.level);
			        }
			        Observer.OnListOfAudioLevelsChangedEvent(AudioLevels);
		        })
		    .on_error(MakeHandler(__LINE__));
	}

	void FSdkAccess::SetInputDevice(const int Index)
	{
		Devices->SetInputDevice(Index);
	}

	void FSdkAccess::SetOutputDevice(const int Index)
	{
		Devices->SetOutputDevice(Index);
	}

	FErrorHandler FSdkAccess::MakeHandler(int Id)
	{
		return FErrorHandler(
		    [this, Id](const FString& Msg)
		    {
			    UE_LOG(LogDolbyIO, Error, TEXT("%s (conference status: %s)"),
			           *(Msg + " {" + FString::FromInt(Id) + "}"), *ToString(ConferenceStatus));
		    },
		    [this]()
		    {
			    if (IsConnected())
			    {
				    Disconnect();
			    }
		    });
	}
}