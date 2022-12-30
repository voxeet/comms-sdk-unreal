// Copyright 2022 Dolby Laboratories

#include "DolbyIO/SdkAccess.h"

#include "DolbyIO/ErrorHandler.h"
#include "DolbyIO/Logging.h"
#include "DolbyIO/SdkEventObserver.h"
#include "DolbyIOParticipantInfo.h"

#include <dolbyio/comms/async_result.h>
#include <dolbyio/comms/sdk.h>

#include "HAL/PlatformProcess.h"
#include "Math/Rotator.h"

namespace DolbyIO
{
	using namespace dolbyio::comms;

	FSdkAccess::FSdkAccess(ISdkEventObserver& Observer)
	    : Observer(Observer), ConferenceStatus(conference_status::destroyed)
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

	// to be removed when fix lands in C++ SDK
	FSdkAccess::~FSdkAccess()
	{
		while (ConferenceStatus < conference_status::left)
		{
			if (IsConnected())
			{
				Disconnect();
			}
			FPlatformProcess::Sleep(0.1);
		}
	}

	namespace
	{
		std::string ToStdString(const FString& String)
		{
			return TCHAR_TO_UTF8(*String);
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
			RefreshTokenCb.Reset(); // RefreshToken callback can only be called once
		}
	}
	catch (...)
	{
		MakeHandler(__LINE__).RethrowAndUpdateStatus();
	}

	namespace
	{
		auto ToUnrealParticipantInfo(const participant_info& Info)
		{
			FDolbyIOParticipantInfo Ret;
			Ret.UserID = Info.user_id.c_str();
			Ret.Name = Info.info.name.value_or("").c_str();
			Ret.ExternalID = Info.info.external_id.value_or("").c_str();
			Ret.AvatarURL = Info.info.avatar_url.value_or("").c_str();
			Ret.bIsListener = Info.type && *Info.type == participant_type::listener;
			Ret.bIsSendingAudio = Info.is_sending_audio.value_or(false);
			Ret.bIsAudibleLocally = Info.audible_locally.value_or(false);
			return Ret;
		}
	}

	void FSdkAccess::Initialize(const FToken& Token)
	{
		Sdk.Reset(sdk::create(ToStdString(Token),
		                      [this](auto&& cb)
		                      {
			                      UE_LOG(LogDolbyIO, Log, TEXT("Refresh token requested"));
			                      RefreshTokenCb.Reset(cb.release());
			                      Observer.OnTokenNeededEvent();
		                      })
		              .release());

		Sdk->conference()
		    .add_event_handler([this](const conference_status_updated& Event) { UpdateStatus(Event.status); })
		    .then(
		        [this](auto)
		        {
			        return Sdk->conference().add_event_handler(
			            [this](const participant_added& Event)
			            {
				            RemoteParticipantIDs.Add(Event.participant.user_id.c_str());
				            Observer.OnParticipantAddedEvent(ToUnrealParticipantInfo(Event.participant));
			            });
		        })
		    .then(
		        [this](auto)
		        {
			        return Sdk->conference().add_event_handler(
			            [this](const participant_updated& Event)
			            {
				            const auto& ParticipantStatus = Event.participant.status;
				            if (ParticipantStatus)
				            {
					            if (*ParticipantStatus == participant_status::left)
					            {
						            RemoteParticipantIDs.Remove(Event.participant.user_id.c_str());
						            Observer.OnParticipantLeftEvent(ToUnrealParticipantInfo(Event.participant));
					            }
				            }
			            });
		        })
		    .then(
		        [this](auto)
		        {
			        return Sdk->conference().add_event_handler(
			            [this](const active_speaker_change& Event)
			            {
				            FParticipantIDs ActiveSpeakers;
				            for (const auto& Speaker : Event.active_speakers)
				            {
					            ActiveSpeakers.Add(Speaker.c_str());
				            }
				            Observer.OnActiveSpeakersChangedEvent(ActiveSpeakers);
			            });
		        })
		    .then([this](auto) { Observer.OnInitializedEvent(); })
		    .on_error(MakeHandler(__LINE__));
	}

	bool FSdkAccess::IsConnected() const
	{
		return ConferenceStatus == conference_status::joined;
	}

	bool FSdkAccess::CanConnect() const
	{
		if (!Sdk)
		{
			UE_LOG(LogDolbyIO, Warning, TEXT("Cannot connect - not initialized"));
			return false;
		}
		if (IsConnected())
		{
			UE_LOG(LogDolbyIO, Warning, TEXT("Cannot connect - already connected, please disconnect first"));
			return false;
		}
		return true;
	}

	namespace
	{
		FString ToString(conference_status Status)
		{
			switch (Status)
			{
				case conference_status::creating:
					return "creating";
				case conference_status::created:
					return "created";
				case conference_status::joining:
					return "joining";
				case conference_status::joined:
					return "joined";
				case conference_status::leaving:
					return "leaving";
				case conference_status::left:
					return "left";
				case conference_status::destroyed:
					return "destroyed";
				case conference_status::error:
					return "error";
				default:
					return "unknown";
			};
		}
	}

	void FSdkAccess::UpdateStatus(conference_status Status)
	{
		ConferenceStatus = Status;
		UE_LOG(LogDolbyIO, Log, TEXT("Conference status: %s"), *ToString(ConferenceStatus));

		switch (ConferenceStatus)
		{
			case conference_status::joined:
				Observer.OnConnectedEvent(LocalParticipantID);
				break;
			case conference_status::left:
			case conference_status::error:
				Observer.OnDisconnectedEvent();
				break;
		}
	}

	void FSdkAccess::Connect(const FString& ConferenceName, const FString& UserName, const FString& ExternalID,
	                         const FString& AvatarURL)
	{
		if (!CanConnect())
		{
			return;
		}
		if (ConferenceName.IsEmpty())
		{
			UE_LOG(LogDolbyIO, Warning, TEXT("Cannot connect - conference name cannot be empty"));
			return;
		}

		RemoteParticipantIDs.Empty();
		bIsDemo = false;

		using namespace dolbyio::comms::services;
		services::session::user_info UserInfo{};
		UserInfo.name = ToStdString(UserName);
		UserInfo.externalId = ToStdString(ExternalID);
		UserInfo.avatarUrl = ToStdString(AvatarURL);

		ConferenceStatus = conference_status::creating; // to be removed when fix lands in C++ SDK
		Sdk->session()
		    .open(MoveTemp(UserInfo))
		    .then(
		        [this, ConferenceName](auto&& User)
		        {
			        if (User.participant_id)
			        {
				        LocalParticipantID = User.participant_id->c_str();
			        }

			        conference::conference_options Options{};
			        Options.alias = ToStdString(ConferenceName);
			        Options.params.spatial_audio_style = spatial_audio_style::shared;

			        return Sdk->conference().create(Options);
		        })
		    .then(
		        [this](auto&& ConferenceInfo)
		        {
			        conference::join_options Options{};
			        Options.constraints.audio = true;
			        Options.connection.spatial_audio = true;
			        return Sdk->conference().join(ConferenceInfo, Options);
		        })
		    .on_error(MakeHandler(__LINE__));
	}

	void FSdkAccess::ConnectToDemoConference()
	{
		if (!CanConnect())
		{
			return;
		}

		RemoteParticipantIDs.Empty();
		bIsDemo = true;

		ConferenceStatus = conference_status::creating; // to be removed when fix lands in C++ SDK
		Sdk->session()
		    .open({})
		    .then(
		        [this](auto&& User)
		        {
			        if (User.participant_id)
			        {
				        LocalParticipantID = User.participant_id->c_str();
			        }
			        return Sdk->conference().demo();
		        })
		    .on_error(MakeHandler(__LINE__));
	}

	void FSdkAccess::Disconnect()
	{
		if (!IsConnected())
		{
			return;
		}

		Sdk->conference().leave().then([this]() { return Sdk->session().close(); }).on_error(MakeHandler(__LINE__));
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
			for (const auto& Participant : RemoteParticipantIDs)
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
			        Observer.OnAudioLevelsChangedEvent(AudioLevels);
		        })
		    .on_error(MakeHandler(__LINE__));
	}

	FErrorHandler FSdkAccess::MakeHandler(int Line)
	{
		return FErrorHandler{[this, Line](const FString& Msg)
		                     {
			                     if (ConferenceStatus != conference_status::leaving)
			                     {
				                     UE_LOG(LogDolbyIO, Error, TEXT("%s (conference status: %s)"),
				                            *(Msg + " {" + FString::FromInt(Line) + "}"), *ToString(ConferenceStatus));
			                     }
		                     }};
	}
}
