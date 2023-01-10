// Copyright 2022 Dolby Laboratories

#include "DolbyIOSdkAccess.h"

#include "DolbyIOErrorHandler.h"
#include "DolbyIOLogging.h"
#include "DolbyIOParticipantInfo.h"
#include "DolbyIOSubsystem.h"

#include <dolbyio/comms/sdk.h>

#include "Async/Async.h"

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
			UE_LOG(LogDolbyIO, Warning, TEXT("Ignoring request to set token when no token is needed"));
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
		                      [this](auto&& RefreshCb)
		                      {
			                      UE_LOG(LogDolbyIO, Log, TEXT("Refresh token requested"));
			                      RefreshTokenCb.Reset(RefreshCb.release());
			                      BroadcastEvent(DolbyIOSubsystem.OnTokenNeeded);
		                      })
		              .release());

		Sdk->conference()
		    .add_event_handler([this](const conference_status_updated& Event) { UpdateStatus(Event.status); })
		    .then(
		        [this](auto)
		        {
			        return Sdk->conference().add_event_handler(
			            [this](const participant_added& Event) {
				            BroadcastEvent(DolbyIOSubsystem.OnParticipantAdded,
				                           ToUnrealParticipantInfo(Event.participant));
			            });
		        })
		    .then(
		        [this](auto)
		        {
			        return Sdk->conference().add_event_handler(
			            [this](const participant_updated& Event)
			            {
				            if (Event.participant.status && *Event.participant.status == participant_status::left)
				            {
					            BroadcastEvent(DolbyIOSubsystem.OnParticipantLeft,
					                           ToUnrealParticipantInfo(Event.participant));
				            }
			            });
		        })
		    .then(
		        [this](auto)
		        {
			        return Sdk->conference().add_event_handler(
			            [this](const active_speaker_change& Event)
			            {
				            TArray<FString> ActiveSpeakers;
				            for (const auto& Speaker : Event.active_speakers)
				            {
					            ActiveSpeakers.Add(Speaker.c_str());
				            }
				            BroadcastEvent(DolbyIOSubsystem.OnActiveSpeakersChanged, ActiveSpeakers);
			            });
		        })
		    .then([this](auto) { BroadcastEvent(DolbyIOSubsystem.OnInitialized); })
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

		switch (ConferenceStatus)
		{
			case conference_status::joined:
				BroadcastEvent(DolbyIOSubsystem.OnConnected, LocalParticipantID.c_str());
				break;
			case conference_status::left:
			case conference_status::error:
				BroadcastEvent(DolbyIOSubsystem.OnDisconnected);
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

		using namespace dolbyio::comms::services;
		services::session::user_info UserInfo{};
		UserInfo.name = ToStdString(UserName);
		UserInfo.externalId = ToStdString(ExternalID);
		UserInfo.avatarUrl = ToStdString(AvatarURL);

		Sdk->session()
		    .open(MoveTemp(UserInfo))
		    .then(
		        [this, ConferenceName = ToStdString(ConferenceName)](auto&& User)
		        {
			        LocalParticipantID = User.participant_id.value_or("");

			        conference::conference_options Options{};
			        Options.alias = ConferenceName;
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
		    .then(
		        [this](auto&&)
		        {
			        SetSpatialEnvironment();
			        ToggleInputMute();
			        ToggleOutputMute();
		        })
		    .on_error(MakeErrorHandler(__LINE__));
	}

	void FSdkAccess::DemoConference()
	{
		if (!CanConnect())
		{
			return;
		}

		Sdk->session()
		    .open({})
		    .then(
		        [this](auto&& User)
		        {
			        LocalParticipantID = User.participant_id.value_or("");
			        return Sdk->conference().demo(spatial_audio_style::shared);
		        })
		    .then(
		        [this](auto&&)
		        {
			        SetSpatialEnvironment();
			        ToggleInputMute();
			        ToggleOutputMute();
		        })
		    .on_error(MakeErrorHandler(__LINE__));
	}

	void FSdkAccess::SetSpatialEnvironment()
	{
		// The SDK spatial settings expect meters as the default unit of length.
		// Unreal uses centimeters for scale, so the plugin's scale of "1" is a scale of "100" for the SDK.
		const auto SdkScale = SpatialEnvironmentScale * 100;
		const spatial_scale Scale{SdkScale, SdkScale, SdkScale};
		const spatial_position Forward{1, 0, 0};
		const spatial_position Up{0, 0, 1};
		const spatial_position Right{0, 1, 0};
		Sdk->conference().set_spatial_environment(Scale, Forward, Up, Right).on_error(MakeErrorHandler(__LINE__));
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

	void FSdkAccess::SetSpatialEnvironmentScale(float Scale)
	{
		SpatialEnvironmentScale = Scale;
		if (IsConnected())
		{
			SetSpatialEnvironment();
		}
	}

	void FSdkAccess::ToggleInputMute()
	{
		if (IsConnected())
		{
			Sdk->conference().mute(bIsInputMuted).on_error(MakeErrorHandler(__LINE__));
		}
	}

	void FSdkAccess::ToggleOutputMute()
	{
		if (IsConnected())
		{
			Sdk->conference().mute_output(bIsOutputMuted).on_error(MakeErrorHandler(__LINE__));
		}
	}

	void FSdkAccess::MuteInput()
	{
		bIsInputMuted = true;
		ToggleInputMute();
	}

	void FSdkAccess::UnmuteInput()
	{
		bIsInputMuted = false;
		ToggleInputMute();
	}

	void FSdkAccess::MuteOutput()
	{
		bIsOutputMuted = true;
		ToggleOutputMute();
	}

	void FSdkAccess::UnmuteOutput()
	{
		bIsOutputMuted = false;
		ToggleOutputMute();
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
			        TArray<FString> ActiveSpeakers;
			        TArray<float> AudioLevels;
			        for (const audio_level& Level : Levels)
			        {
				        ActiveSpeakers.Add(Level.participant_id.c_str());
				        AudioLevels.Add(Level.level);
			        }
			        BroadcastEvent(DolbyIOSubsystem.OnAudioLevelsChanged, ActiveSpeakers, AudioLevels);
		        })
		    .on_error(MakeErrorHandler(__LINE__));
	}

	void FSdkAccess::UpdateViewPoint(const FVector& Position, const FRotator& Rotation)
	{
		if (!IsConnected())
		{
			return;
		}

		spatial_audio_batch_update Update;
		Update.set_spatial_position(LocalParticipantID, {Position.X, Position.Y, Position.Z});
		// The SDK expects the direction values to mean rotations around the {x,y,z} axes as specified by the
		// environment. In Unreal, rotation around x is roll (because x is forward), y is pitch and z is yaw.
		Update.set_spatial_direction({Rotation.Roll, Rotation.Pitch, Rotation.Yaw});
		Sdk->conference().update_spatial_audio_configuration(MoveTemp(Update)).on_error(MakeErrorHandler(__LINE__));
	}

	FErrorHandler FSdkAccess::MakeErrorHandler(int Line)
	{
		return {[this, Line](const FString& Msg)
		        {
			        UE_LOG(LogDolbyIO, Error, TEXT("%s (conference status: %s)"),
			               *(Msg + " {" + FString::FromInt(Line) + "}"), *ToString(ConferenceStatus));
		        }};
	}

	template <class TDelegate, class... TArgs> void FSdkAccess::BroadcastEvent(TDelegate& Event, TArgs&&... Args)
	{
		if (bIsAlive)
		{
			AsyncTask(ENamedThreads::GameThread, [=] { Event.Broadcast(Args...); });
		}
	}
}
