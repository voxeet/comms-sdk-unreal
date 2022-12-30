// Copyright 2022 Dolby Laboratories

#include "DolbyIO/SdkAccess.h"

#include "DolbyIO/ErrorHandler.h"
#include "DolbyIO/Logging.h"
#include "DolbyIOParticipantInfo.h"
#include "DolbyIOSubsystem.h"

#include <dolbyio/comms/async_result.h>
#include <dolbyio/comms/sdk.h>

#include "Async/Async.h"
#include "HAL/PlatformProcess.h" // to be removed when fix lands in C++ SDK
#include "Math/Rotator.h"

namespace DolbyIO
{
	using namespace dolbyio::comms;

	FSdkAccess::FSdkAccess(UDolbyIOSubsystem& DolbyIOSubsystem)
	    : DolbyIOSubsystem(DolbyIOSubsystem), ConferenceStatus(conference_status::destroyed)
	{
		static std::once_flag DoOnce;
		std::call_once(DoOnce,
		               []
		               {
#if PLATFORM_WINDOWS
			               static auto AlignedNew = +[](std::size_t Count, std::size_t Al)
			               { return operator new(Count, static_cast<std::align_val_t>(Al)); };
			               static auto AlignedDelete = +[](void* Ptr, std::size_t Al)
			               { operator delete(Ptr, static_cast<std::align_val_t>(Al)); };
			               sdk::set_app_allocator({operator new, AlignedNew, operator delete, AlignedDelete});
#endif
		               });
	}

	FSdkAccess::~FSdkAccess()
	{
		bIsAlive = false;
		while (ConferenceStatus < conference_status::left) // to be removed when fix lands in C++ SDK
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

	void FSdkAccess::SetToken(const FString& Token)
	try
	{
		if (!Sdk)
		{
			Initialize(Token);
		}
		else if (RefreshTokenCb)
		{
			(*RefreshTokenCb)(ToStdString(Token));
			RefreshTokenCb.Reset(); // RefreshToken callback can only be called once
		}
		else
		{
			UE_LOG(LogDolbyIO, Log, TEXT("Ignoring request to set token when no token is needed"));
		}
	}
	catch (...)
	{
		MakeErrorHandler(__LINE__).HandleError();
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

	void FSdkAccess::Initialize(const FString& Token)
	{
		Sdk.Reset(sdk::create(ToStdString(Token),
		                      [this](auto&& cb)
		                      {
			                      if (!bIsAlive)
			                      {
				                      return;
			                      }

			                      UE_LOG(LogDolbyIO, Log, TEXT("Refresh token requested"));
			                      RefreshTokenCb.Reset(cb.release());
			                      AsyncTask(ENamedThreads::GameThread,
			                                [=] { DolbyIOSubsystem.OnTokenNeeded.Broadcast(); });
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
				            if (!bIsAlive)
				            {
					            return;
				            }

				            RemoteParticipantIDs.Add(Event.participant.user_id.c_str());
				            AsyncTask(ENamedThreads::GameThread,
				                      [=] {
					                      DolbyIOSubsystem.OnParticipantAdded.Broadcast(
					                          ToUnrealParticipantInfo(Event.participant));
				                      });
			            });
		        })
		    .then(
		        [this](auto)
		        {
			        return Sdk->conference().add_event_handler(
			            [this](const participant_updated& Event)
			            {
				            if (!bIsAlive)
				            {
					            return;
				            }

				            const auto& ParticipantStatus = Event.participant.status;
				            if (ParticipantStatus)
				            {
					            if (*ParticipantStatus == participant_status::left)
					            {
						            RemoteParticipantIDs.Remove(Event.participant.user_id.c_str());
						            AsyncTask(ENamedThreads::GameThread,
						                      [=] {
							                      DolbyIOSubsystem.OnParticipantLeft.Broadcast(
							                          ToUnrealParticipantInfo(Event.participant));
						                      });
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
				            if (!bIsAlive)
				            {
					            return;
				            }

				            TArray<FString> ActiveSpeakers;
				            for (const auto& Speaker : Event.active_speakers)
				            {
					            ActiveSpeakers.Add(Speaker.c_str());
				            }
				            AsyncTask(ENamedThreads::GameThread,
				                      [=] { DolbyIOSubsystem.OnActiveSpeakersChanged.Broadcast(ActiveSpeakers); });
			            });
		        })
		    .then(
		        [this](auto)
		        {
			        if (!bIsAlive)
			        {
				        return;
			        }

			        AsyncTask(ENamedThreads::GameThread, [=] { DolbyIOSubsystem.OnInitialized.Broadcast(); });
		        })
		    .on_error(MakeErrorHandler(__LINE__));
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

		if (!bIsAlive)
		{
			return;
		}

		switch (ConferenceStatus)
		{
			case conference_status::joined:
				AsyncTask(ENamedThreads::GameThread,
				          [=] { DolbyIOSubsystem.OnConnected.Broadcast(LocalParticipantID); });
				break;
			case conference_status::left:
			case conference_status::error:
				AsyncTask(ENamedThreads::GameThread, [=] { DolbyIOSubsystem.OnDisconnected.Broadcast(); });
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
		    .on_error(MakeErrorHandler(__LINE__));
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
		    .on_error(MakeErrorHandler(__LINE__));
	}

	void FSdkAccess::Disconnect()
	{
		if (!IsConnected())
		{
			return;
		}

		Sdk->conference()
		    .leave()
		    .then([this]() { return Sdk->session().close(); })
		    .on_error(MakeErrorHandler(__LINE__));
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
		Sdk->conference().update_spatial_audio_configuration(MoveTemp(Update)).on_error(MakeErrorHandler(__LINE__));
	}

	void FSdkAccess::MuteInput()
	{
		if (!IsConnected())
		{
			UE_LOG(LogDolbyIO, Warning, TEXT("Must be connected to mute input"));
			return;
		}

		Sdk->conference().mute(true).on_error(MakeErrorHandler(__LINE__));
	}

	void FSdkAccess::UnmuteInput()
	{
		if (!IsConnected())
		{
			return;
		}

		Sdk->conference().mute(false).on_error(MakeErrorHandler(__LINE__));
	}

	void FSdkAccess::MuteOutput()
	{
		if (!IsConnected())
		{
			UE_LOG(LogDolbyIO, Warning, TEXT("Must be connected to mute output"));
			return;
		}

		Sdk->conference().mute_output(true).on_error(MakeErrorHandler(__LINE__));
	}

	void FSdkAccess::UnmuteOutput()
	{
		if (!IsConnected())
		{
			return;
		}

		Sdk->conference().mute_output(false).on_error(MakeErrorHandler(__LINE__));
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
			        if (!bIsAlive)
			        {
				        return;
			        }

			        TArray<FString> ActiveSpeakers;
			        TArray<float> AudioLevels;
			        for (const audio_level& Level : Levels)
			        {
				        ActiveSpeakers.Add(Level.participant_id.c_str());
				        AudioLevels.Add(Level.level);
			        }
			        AsyncTask(ENamedThreads::GameThread,
			                  [=] { DolbyIOSubsystem.OnAudioLevelsChanged.Broadcast(ActiveSpeakers, AudioLevels); });
		        })
		    .on_error(MakeErrorHandler(__LINE__));
	}

	FErrorHandler FSdkAccess::MakeErrorHandler(int Line)
	{
		return {[this, Line](const FString& Msg)
		        {
			        UE_LOG(LogDolbyIO, Error, TEXT("%s (conference status: %s)"),
			               *(Msg + " {" + FString::FromInt(Line) + "}"), *ToString(ConferenceStatus));
		        }};
	}
}
