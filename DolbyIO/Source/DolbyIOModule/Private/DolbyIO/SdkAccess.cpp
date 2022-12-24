// Copyright 2022 Dolby Laboratories

#include "DolbyIO/SdkAccess.h"

#include "DolbyIO/DeviceManagement.h"
#include "DolbyIO/ErrorHandler.h"
#include "DolbyIO/Logging.h"
#include "DolbyIO/SdkEventObserver.h"
#include "DolbyIO/SdkStatus.h"

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

	FSdkAccess::FSdkAccess(ISdkEventObserver& Observer) : Observer(Observer), Status(MakeUnique<FSdkStatus>(Observer))
	{
	}

	FSdkAccess::~FSdkAccess()
	{
		if (Status->IsConnected())
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
			                      DLB_UE_LOG("Refresh token requested");
			                      RefreshTokenCb.Reset(cb.release());
			                      Observer.OnTokenNeededEvent();
		                      })
		              .release());
		Devices.Reset(MakeUnique<FDeviceManagement>(Sdk->device_management(), Observer,
		                                            [this](int Id) { return MakeHandler(Id); })
		                  .Release());

		Sdk->conference()
		    .add_event_handler([this](const conference_status_updated& Event) { Status->SetStatus(Event.status); })
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

	void FSdkAccess::Connect(const FConferenceName& Conf, const FUserName& User)
	try
	{
		if (!Sdk)
		{
			DLB_UE_LOG("Cannot connect - not initialized");
			return;
		}
		if (Status->IsConnected())
		{
			DLB_UE_LOG("Cannot connect - already connected, please disconnect first");
			return;
		}
		if (Conf.IsEmpty())
		{
			DLB_UE_LOG("Cannot connect - conference name cannot be empty");
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
		if (!Status->IsConnected())
		{
			DLB_UE_LOG("Cannot disconnect - not connected");
			return;
		}

		LocalParticipantID.Reset();
		ParticipantIDs.Empty();
		Sdk->conference().leave().then([this]() { return Sdk->session().close(); }).on_error(MakeHandler(__LINE__));
	}

	void FSdkAccess::UpdateViewPoint(const FVector& Position, const FRotator& Rotation)
	{
		if (!Status->IsConnected())
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
		if (!Status->IsConnected())
		{
			DLB_UE_LOG("Cannot mute input - not connected");
			return;
		}

		Sdk->conference().mute(true).on_error(MakeHandler(__LINE__));
	}

	void FSdkAccess::UnmuteInput()
	{
		if (!Status->IsConnected())
		{
			DLB_UE_LOG("Cannot unmute input - not connected");
			return;
		}

		Sdk->conference().mute(false).on_error(MakeHandler(__LINE__));
	}

	void FSdkAccess::MuteOutput()
	{
		if (!Status->IsConnected())
		{
			DLB_UE_LOG("Cannot mute output - not connected");
			return;
		}

		Sdk->conference().mute_output(true).on_error(MakeHandler(__LINE__));
	}

	void FSdkAccess::UnmuteOutput()
	{
		if (!Status->IsConnected())
		{
			DLB_UE_LOG("Cannot unmute output - not connected");
			return;
		}

		Sdk->conference().mute_output(false).on_error(MakeHandler(__LINE__));
	}

	void FSdkAccess::GetAudioLevels()
	{
		if (!Status->IsConnected())
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
		return FErrorHandler([this, Id](const FString& Msg)
		                     { Status->Log(Msg + " {" + std::to_string(Id).c_str() + "}"); },
		                     [this]()
		                     {
			                     if (Status->IsConnected())
			                     {
				                     Disconnect();
			                     }
		                     });
	}
}
