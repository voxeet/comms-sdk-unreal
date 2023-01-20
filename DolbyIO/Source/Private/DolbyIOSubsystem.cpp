// Copyright 2023 Dolby Laboratories

#include "DolbyIOSubsystem.h"

#include "DolbyIOLogging.h"
#include "DolbyIOVideoSink.h"

#if PLATFORM_MAC
#define DOLBYIO_COMMS_SUPPRESS_APPLE_NO_RTTI_WARNING
#endif
#include <dolbyio/comms/sdk.h>

#include "Async/Async.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Misc/Base64.h"
#include "TimerManager.h"

void UDolbyIOSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

#if PLATFORM_WINDOWS
	dolbyio::comms::sdk::set_app_allocator(
	    {::operator new,
	     [](std::size_t Count, std::size_t Al) { return ::operator new(Count, static_cast<std::align_val_t>(Al)); },
	     ::operator delete,
	     [](void* Ptr, std::size_t Al) { ::operator delete(Ptr, static_cast<std::align_val_t>(Al)); }});
#endif

	ConferenceStatus = dolbyio::comms::conference_status::destroyed;

	VideoSink = MakeShared<DolbyIO::FVideoSink>();

	FTimerManager& TimerManager = GetGameInstance()->GetTimerManager();
	TimerManager.SetTimer(LocationTimerHandle, this, &UDolbyIOSubsystem::SetLocationUsingFirstPlayer, 0.1, true);
	TimerManager.SetTimer(RotationTimerHandle, this, &UDolbyIOSubsystem::SetRotationUsingFirstPlayer, 0.01, true);

	OnTokenNeeded.Broadcast();
}

void UDolbyIOSubsystem::Deinitialize()
{
	Sdk.Reset(); // make sure Sdk is dead so it doesn't call handle_frame on VideoSink during game destruction
	Super::Deinitialize();
}

namespace
{
	std::string ToStdString(const FString& String)
	{
		return TCHAR_TO_UTF8(*String);
	}

	constexpr int ScaleCenti = 100;

	FDolbyIOParticipantInfo ToUnrealParticipantInfo(const dolbyio::comms::participant_info& Info)
	{
		FDolbyIOParticipantInfo Ret{};
		Ret.UserID = Info.user_id.c_str();
		Ret.Name = Info.info.name.value_or("").c_str();
		Ret.ExternalID = Info.info.external_id.value_or("").c_str();
		Ret.AvatarURL = Info.info.avatar_url.value_or("").c_str();
		Ret.bIsListener = Info.type && *Info.type == dolbyio::comms::participant_type::listener;
		Ret.bIsSendingAudio = Info.is_sending_audio.value_or(false);
		Ret.bIsAudibleLocally = Info.audible_locally.value_or(false);

		FString DecodedExternalID;
		FBase64::Decode(Ret.ExternalID, DecodedExternalID);
		if (DecodedExternalID.StartsWith("{\"init-pos\": {\"x\": "))
		{
			Ret.bIsInjectedBot = true;

			FString Left;
			FString Right;

			DecodedExternalID.Split("\"x\": ", &Left, &Right);
			Right.Split(",", &Left, &Right);
			Ret.Location.Y = FCString::Atof(*Left) * ScaleCenti;

			Right.Split("\"y\": ", &Left, &Right);
			Right.Split(",", &Left, &Right);
			Ret.Location.Z = FCString::Atof(*Left) * ScaleCenti;

			Right.Split("\"z\": ", &Left, &Right);
			Right.Split(",", &Left, &Right);
			Ret.Location.X = -FCString::Atof(*Left) * ScaleCenti;

			Right.Split("\"r\": ", &Left, &Right);
			Right.Split(",", &Left, &Right);
			Ret.Rotation.Yaw = FCString::Atof(*Left);

			Ret.Rotation.Roll = 0;
			Ret.Rotation.Pitch = 0;
		}
		return Ret;
	}

	FString ToString(dolbyio::comms::conference_status Status)
	{
		switch (Status)
		{
			case dolbyio::comms::conference_status::creating:
				return "creating";
			case dolbyio::comms::conference_status::created:
				return "created";
			case dolbyio::comms::conference_status::joining:
				return "joining";
			case dolbyio::comms::conference_status::joined:
				return "joined";
			case dolbyio::comms::conference_status::leaving:
				return "leaving";
			case dolbyio::comms::conference_status::left:
				return "left";
			case dolbyio::comms::conference_status::destroyed:
				return "destroyed";
			case dolbyio::comms::conference_status::error:
				return "error";
			default:
				return "unknown";
		};
	}

	FString ToString(dolbyio::comms::participant_status Status)
	{
		switch (Status)
		{
			case dolbyio::comms::participant_status::reserved:
				return "reserved";
			case dolbyio::comms::participant_status::connecting:
				return "connecting";
			case dolbyio::comms::participant_status::on_air:
				return "on_air";
			case dolbyio::comms::participant_status::decline:
				return "decline";
			case dolbyio::comms::participant_status::inactive:
				return "inactive";
			case dolbyio::comms::participant_status::left:
				return "left";
			case dolbyio::comms::participant_status::warning:
				return "warning";
			case dolbyio::comms::participant_status::error:
				return "error";
			default:
				return "unknown";
		};
	}

	class FErrorHandler final
	{
	public:
		FErrorHandler(dolbyio::comms::conference_status ConferenceStatus, int Line)
		    : ConferenceStatus(ConferenceStatus), Line(Line)
		{
		}
#define DLB_ERROR_HANDLER FErrorHandler(ConferenceStatus, __LINE__)

		void operator()(std::exception_ptr&& ExcPtr) const
		{
			HandleError([ExcP = MoveTemp(ExcPtr)] { std::rethrow_exception(ExcP); });
		}

		void HandleError() const
		{
			HandleError([] { throw; });
		}

	private:
		void HandleError(TFunction<void()> Callee) const
		try
		{
			Callee();
		}
		catch (const dolbyio::comms::conference_state_exception& Ex)
		{
			LogException("dolbyio::comms::conference_state_exception", Ex.what());
		}
		catch (const dolbyio::comms::invalid_token_exception& Ex)
		{
			LogException("dolbyio::comms::invalid_token_exception", Ex.what());
		}
		catch (const dolbyio::comms::dvc_error_exception& Ex)
		{
			LogException("dolbyio::comms::dvc_error_exception", Ex.what());
		}
		catch (const dolbyio::comms::peer_connection_failed_exception& Ex)
		{
			LogException("dolbyio::comms::peer_connection_failed_exception", Ex.what());
		}
		catch (const dolbyio::comms::exception& Ex)
		{
			LogException("dolbyio::comms::exception", Ex.what());
		}
		catch (const std::exception& Ex)
		{
			LogException("std::exception", Ex.what());
		}
		catch (...)
		{
			LogException("unknown exception", "");
		}

		void LogException(const FString& Type, const FString& What) const
		{
			DLB_UE_LOG(Error, "Caught %s: %s (conference status: %s, line: %d)", *Type, *What,
			           *ToString(ConferenceStatus), Line);
		}

		dolbyio::comms::conference_status ConferenceStatus;
		int Line;
	};
}

void UDolbyIOSubsystem::SetToken(const FString& Token)
try
{
	if (!Sdk)
	{
		DLB_UE_LOG(Log, "Initializing with token: %s", *Token);
		Initialize(Token);
	}
	else if (RefreshTokenCb)
	{
		DLB_UE_LOG(Log, "Refreshing token: %s", *Token);
		(*RefreshTokenCb)(ToStdString(Token));
		RefreshTokenCb.Reset(); // RefreshToken callback can only be called once
	}
}
catch (...)
{
	DLB_ERROR_HANDLER.HandleError();
}

void UDolbyIOSubsystem::Initialize(const FString& Token)
{
	Sdk = TSharedPtr<dolbyio::comms::sdk>(
	    dolbyio::comms::sdk::create(ToStdString(Token),
	                                [this](std::unique_ptr<dolbyio::comms::refresh_token>&& RefreshCb)
	                                {
		                                DLB_UE_LOG(Log, "Refresh token requested");
		                                RefreshTokenCb = TSharedPtr<dolbyio::comms::refresh_token>(RefreshCb.release());
		                                BroadcastEvent(OnTokenNeeded);
	                                })
	        .release());

	Sdk->conference()
	    .add_event_handler([this](const dolbyio::comms::conference_status_updated& Event)
	                       { UpdateStatus(Event.status); })
	    .then(
	        [this](dolbyio::comms::event_handler_id)
	        {
		        return Sdk->conference().add_event_handler(
		            [this](const dolbyio::comms::participant_updated& Event)
		            {
			            if (!Event.participant.status)
			            {
				            return;
			            }

			            const FDolbyIOParticipantInfo Info = ToUnrealParticipantInfo(Event.participant);
			            DLB_UE_LOG(Log, "Participant status updated: UserID=%s Name=%s ExternalID=%s Status=%s",
			                       *Info.UserID, *Info.Name, *Info.ExternalID, *ToString(*Event.participant.status));

			            if (Info.UserID == LocalParticipantID)
			            {
				            return;
			            }

			            switch (*Event.participant.status)
			            {
				            case dolbyio::comms::participant_status::on_air:
					            return BroadcastEvent(OnParticipantAdded, Info);
				            case dolbyio::comms::participant_status::left:
					            return BroadcastEvent(OnParticipantLeft, Info);
			            }
		            });
	        })
	    .then(
	        [this](dolbyio::comms::event_handler_id)
	        {
		        return Sdk->conference().add_event_handler(
		            [this](const dolbyio::comms::active_speaker_change& Event)
		            {
			            TArray<FString> ActiveSpeakers;
			            for (const std::string& Speaker : Event.active_speakers)
			            {
				            ActiveSpeakers.Add(Speaker.c_str());
			            }
			            BroadcastEvent(OnActiveSpeakersChanged, ActiveSpeakers);
		            });
	        })
	    .then(
	        [this](dolbyio::comms::event_handler_id)
	        {
		        return Sdk->conference().add_event_handler(
		            [this](const dolbyio::comms::audio_levels& Event)
		            {
			            TArray<FString> ActiveSpeakers;
			            TArray<float> AudioLevels;
			            for (const dolbyio::comms::audio_level& Level : Event.levels)
			            {
				            ActiveSpeakers.Add(Level.participant_id.c_str());
				            AudioLevels.Add(Level.level);
			            }
			            BroadcastEvent(OnAudioLevelsChanged, ActiveSpeakers, AudioLevels);
		            });
	        })
	    .then(
	        [this](dolbyio::comms::event_handler_id)
	        {
		        return Sdk->conference().add_event_handler(
		            [this](const dolbyio::comms::video_track_added& Event)
		            {
			            if (Event.remote)
			            {
				            VideoSink->AddStream(Event.peer_id.c_str(), Event.stream_id.c_str());
			            }
		            });
	        })
	    .then(
	        [this](dolbyio::comms::event_handler_id)
	        {
		        return Sdk->conference().add_event_handler(
		            [this](const dolbyio::comms::video_track_removed& Event)
		            {
			            if (Event.remote)
			            {
				            VideoSink->RemoveStream(Event.peer_id.c_str(), Event.stream_id.c_str());
			            }
		            });
	        })
	    .then([this](dolbyio::comms::event_handler_id)
	          { return Sdk->video().remote().set_video_sink(VideoSink.Get()); })
	    .then(
	        [this]
	        {
		        DLB_UE_LOG(Log, "Initialized");
		        BroadcastEvent(OnInitialized);
	        })
	    .on_error(DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::UpdateStatus(dolbyio::comms::conference_status Status)
{
	ConferenceStatus = Status;
	DLB_UE_LOG(Log, "Conference status: %s", *ToString(ConferenceStatus));

	switch (ConferenceStatus)
	{
		case dolbyio::comms::conference_status::joined:
			BroadcastEvent(OnConnected, LocalParticipantID);
			break;
		case dolbyio::comms::conference_status::left:
		case dolbyio::comms::conference_status::error:
			BroadcastEvent(OnDisconnected);
			break;
	}
}

void UDolbyIOSubsystem::Connect(const FString& ConferenceName, const FString& UserName, const FString& ExternalID,
                                const FString& AvatarURL)
{
	if (!CanConnect())
	{
		return;
	}
	if (ConferenceName.IsEmpty())
	{
		DLB_UE_LOG(Warning, "Cannot connect - conference name cannot be empty");
		return;
	}

	DLB_UE_LOG(Log, "Connecting to conference %s as %s", *ConferenceName, *UserName);

	dolbyio::comms::services::session::user_info UserInfo{};
	UserInfo.name = ToStdString(UserName);
	UserInfo.externalId = ToStdString(ExternalID);
	UserInfo.avatarUrl = ToStdString(AvatarURL);

	Sdk->session()
	    .open(MoveTemp(UserInfo))
	    .then(
	        [this, ConferenceName = ToStdString(ConferenceName)](dolbyio::comms::services::session::user_info&& User)
	        {
		        LocalParticipantID = User.participant_id.value_or("").c_str();

		        dolbyio::comms::services::conference::conference_options Options{};
		        Options.alias = ConferenceName;
		        Options.params.spatial_audio_style = dolbyio::comms::spatial_audio_style::shared;
		        return Sdk->conference().create(Options);
	        })
	    .then(
	        [this](dolbyio::comms::conference_info&& ConferenceInfo)
	        {
		        dolbyio::comms::services::conference::join_options Options{};
		        Options.constraints.audio = true;
		        Options.connection.spatial_audio = true;
		        return Sdk->conference().join(ConferenceInfo, Options);
	        })
	    .then(
	        [this](dolbyio::comms::conference_info&& ConferenceInfo)
	        {
		        DLB_UE_LOG(Log, "Connected to conference ID %s", *FString{ConferenceInfo.id.c_str()});
		        SetSpatialEnvironment();
		        ToggleInputMute();
		        ToggleOutputMute();

		        if (bIsVideoEnabled)
		        {
			        Sdk->video().local().start().on_error(DLB_ERROR_HANDLER);
		        }
	        })
	    .on_error(DLB_ERROR_HANDLER);
}

bool UDolbyIOSubsystem::CanConnect() const
{
	if (!Sdk)
	{
		DLB_UE_LOG(Warning, "Cannot connect - not initialized");
		return false;
	}
	if (IsConnected())
	{
		DLB_UE_LOG(Warning, "Cannot connect - already connected, please disconnect first");
		return false;
	}
	return true;
}

bool UDolbyIOSubsystem::IsConnected() const
{
	return ConferenceStatus == dolbyio::comms::conference_status::joined;
}

void UDolbyIOSubsystem::SetSpatialEnvironment()
{
	// The SDK spatial settings expect meters as the default unit of length.
	// Unreal uses centimeters for scale, so the plugin's scale of "1" is a scale of "100" for the SDK.
	const float SdkScale = SpatialEnvironmentScale * ScaleCenti;
	const dolbyio::comms::spatial_scale Scale{SdkScale, SdkScale, SdkScale};
	const dolbyio::comms::spatial_position Forward{1, 0, 0};
	const dolbyio::comms::spatial_position Up{0, 0, 1};
	const dolbyio::comms::spatial_position Right{0, 1, 0};
	Sdk->conference().set_spatial_environment(Scale, Forward, Up, Right).on_error(DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::ToggleInputMute()
{
	if (IsConnected())
	{
		Sdk->conference().mute(bIsInputMuted).on_error(DLB_ERROR_HANDLER);
	}
}

void UDolbyIOSubsystem::ToggleOutputMute()
{
	if (IsConnected())
	{
		Sdk->conference().mute_output(bIsOutputMuted).on_error(DLB_ERROR_HANDLER);
	}
}

void UDolbyIOSubsystem::ToggleVideo()
{
	if (IsConnected())
	{
		bIsVideoEnabled ? Sdk->video().local().start().on_error(DLB_ERROR_HANDLER)
		                : Sdk->video().local().stop().on_error(DLB_ERROR_HANDLER);
	}
}

void UDolbyIOSubsystem::DemoConference()
{
	if (!CanConnect())
	{
		return;
	}

	DLB_UE_LOG(Log, "Connecting to demo conference");

	Sdk->session()
	    .open({})
	    .then(
	        [this](dolbyio::comms::services::session::user_info&& User)
	        {
		        LocalParticipantID = User.participant_id.value_or("").c_str();
		        return Sdk->conference().demo(dolbyio::comms::spatial_audio_style::shared);
	        })
	    .then(
	        [this](dolbyio::comms::conference_info&& ConferenceInfo)
	        {
		        DLB_UE_LOG(Log, "Connected to conference ID %s", *FString{ConferenceInfo.id.c_str()});
		        SetSpatialEnvironment();
		        ToggleInputMute();
		        ToggleOutputMute();
	        })
	    .on_error(DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::Disconnect()
{
	if (!IsConnected())
	{
		return;
	}

	DLB_UE_LOG(Log, "Disconnecting");
	Sdk->conference().leave().then([this]() { return Sdk->session().close(); }).on_error(DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::SetSpatialEnvironmentScale(float Scale)
{
	DLB_UE_LOG(Log, "Setting spatial environment scale: %f", Scale);
	SpatialEnvironmentScale = Scale;
	if (IsConnected())
	{
		SetSpatialEnvironment();
	}
}

void UDolbyIOSubsystem::MuteInput()
{
	DLB_UE_LOG(Log, "Muting input");
	bIsInputMuted = true;
	ToggleInputMute();
}

void UDolbyIOSubsystem::UnmuteInput()
{
	DLB_UE_LOG(Log, "Unmuting input");
	bIsInputMuted = false;
	ToggleInputMute();
}

void UDolbyIOSubsystem::MuteOutput()
{
	DLB_UE_LOG(Log, "Muting output");
	bIsOutputMuted = true;
	ToggleOutputMute();
}

void UDolbyIOSubsystem::UnmuteOutput()
{
	DLB_UE_LOG(Log, "Unmuting output");
	bIsOutputMuted = false;
	ToggleOutputMute();
}

void UDolbyIOSubsystem::EnableVideo()
{
	DLB_UE_LOG(Log, "Enabling video");
	bIsVideoEnabled = true;
	ToggleVideo();
}

void UDolbyIOSubsystem::DisableVideo()
{
	DLB_UE_LOG(Log, "Disabling video");
	bIsVideoEnabled = false;
	ToggleVideo();
}

UTexture2D* UDolbyIOSubsystem::GetTexture(const FString& ParticipantID)
{
	return VideoSink->GetTexture(ParticipantID);
}

void UDolbyIOSubsystem::SetLocalPlayerLocation(const FVector& Location)
{
	if (LocationTimerHandle.IsValid())
	{
		DLB_UE_LOG(Log, "Disabling automatic location setting");
		GetGameInstance()->GetTimerManager().ClearTimer(LocationTimerHandle);
	}
	SetLocalPlayerLocationImpl(Location);
}

void UDolbyIOSubsystem::SetLocalPlayerLocationImpl(const FVector& Location)
{
	if (!IsConnected())
	{
		return;
	}

	Sdk->conference()
	    .set_spatial_position(ToStdString(LocalParticipantID), {Location.X, Location.Y, Location.Z})
	    .on_error(DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::SetLocalPlayerRotation(const FRotator& Rotation)
{
	if (RotationTimerHandle.IsValid())
	{
		DLB_UE_LOG(Log, "Disabling automatic rotation setting");
		GetGameInstance()->GetTimerManager().ClearTimer(RotationTimerHandle);
	}
	SetLocalPlayerRotationImpl(Rotation);
}

void UDolbyIOSubsystem::SetLocalPlayerRotationImpl(const FRotator& Rotation)
{
	if (!IsConnected())
	{
		return;
	}

	// The SDK expects the direction values to mean rotations around the {x,y,z} axes as specified by the
	// environment. In Unreal, rotation around x is roll (because x is forward), y is pitch and z is yaw.
	Sdk->conference().set_spatial_direction({Rotation.Roll, Rotation.Pitch, Rotation.Yaw}).on_error(DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::SetLocationUsingFirstPlayer()
{
	if (UWorld* World = GetGameInstance()->GetWorld())
	{
		if (APlayerController* FirstPlayerController = World->GetFirstPlayerController())
		{
			if (APawn* Pawn = FirstPlayerController->GetPawn())
			{
				SetLocalPlayerLocationImpl(Pawn->GetActorLocation());
			}
		}
	}
}

void UDolbyIOSubsystem::SetRotationUsingFirstPlayer()
{
	if (UWorld* World = GetGameInstance()->GetWorld())
	{
		if (APlayerController* FirstPlayerController = World->GetFirstPlayerController())
		{
			if (APawn* Pawn = FirstPlayerController->GetPawn())
			{
				SetLocalPlayerRotationImpl(Pawn->GetActorRotation());
			}
		}
	}
}

template <class TDelegate, class... TArgs> void UDolbyIOSubsystem::BroadcastEvent(TDelegate& Event, TArgs&&... Args)
{
	if (IsValid(this))
	{
		AsyncTask(ENamedThreads::GameThread, [=] { Event.Broadcast(Args...); });
	}
}
