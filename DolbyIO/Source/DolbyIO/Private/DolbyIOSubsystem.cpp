// Copyright 2023 Dolby Laboratories

#include "DolbyIOSubsystem.h"

#include "DolbyIOLogging.h"

#if PLATFORM_MAC
#define DOLBYIO_COMMS_SUPPRESS_APPLE_NO_RTTI_WARNING
#endif
#include <dolbyio/comms/sdk.h>

#include "Async/Async.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
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

	FTimerManager& TimerManager = GetGameInstance()->GetTimerManager();
	TimerManager.SetTimer(LocationTimerHandle, this, &UDolbyIOSubsystem::SetLocationUsingFirstPlayer, 0.1, true);
	TimerManager.SetTimer(RotationTimerHandle, this, &UDolbyIOSubsystem::SetRotationUsingFirstPlayer, 0.01, true);

	OnTokenNeeded.Broadcast();
}

void UDolbyIOSubsystem::Deinitialize()
{
	bIsAlive = false;

	Super::Deinitialize();
}

namespace
{
	std::string ToStdString(const FString& String)
	{
		return TCHAR_TO_UTF8(*String);
	}

	FDolbyIOParticipantInfo ToUnrealParticipantInfo(const dolbyio::comms::participant_info& Info)
	{
		FDolbyIOParticipantInfo Ret;
		Ret.UserID = Info.user_id.c_str();
		Ret.Name = Info.info.name.value_or("").c_str();
		Ret.ExternalID = Info.info.external_id.value_or("").c_str();
		Ret.AvatarURL = Info.info.avatar_url.value_or("").c_str();
		Ret.bIsListener = Info.type && *Info.type == dolbyio::comms::participant_type::listener;
		Ret.bIsSendingAudio = Info.is_sending_audio.value_or(false);
		Ret.bIsAudibleLocally = Info.audible_locally.value_or(false);
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

	class FErrorHandler final
	{
	public:
		FErrorHandler(dolbyio::comms::conference_status ConferenceStatus, int Line)
		    : ConferenceStatus(ConferenceStatus), Line(Line)
		{
		}
#define DLB_ERROR_HANDLER FErrorHandler(ConferenceStatus, __LINE__)

		void operator()(std::exception_ptr&& ExcPtr)
		{
			HandleError([ExcP = MoveTemp(ExcPtr)] { std::rethrow_exception(ExcP); });
		}

		void HandleError()
		{
			HandleError([] { throw; });
		}

	private:
		void HandleError(TFunction<void()> Callee)
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

		void LogException(const FString& Type, const FString& What)
		{
			UE_LOG(LogDolbyIO, Error, TEXT("Caught %s: %s (conference status: %s, line: %d)"), *Type, *What,
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
	DLB_ERROR_HANDLER.HandleError();
}

void UDolbyIOSubsystem::Initialize(const FString& Token)
{
	Sdk = TSharedPtr<dolbyio::comms::sdk>(
	    dolbyio::comms::sdk::create(ToStdString(Token),
	                                [this](std::unique_ptr<dolbyio::comms::refresh_token>&& RefreshCb)
	                                {
		                                UE_LOG(LogDolbyIO, Log, TEXT("Refresh token requested"));
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
		            [this](const dolbyio::comms::participant_added& Event)
		            { BroadcastEvent(OnParticipantAdded, ToUnrealParticipantInfo(Event.participant)); });
	        })
	    .then(
	        [this](dolbyio::comms::event_handler_id)
	        {
		        return Sdk->conference().add_event_handler(
		            [this](const dolbyio::comms::participant_updated& Event)
		            {
			            if (Event.participant.status &&
			                *Event.participant.status == dolbyio::comms::participant_status::left)
			            {
				            BroadcastEvent(OnParticipantLeft, ToUnrealParticipantInfo(Event.participant));
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
	    .then([this](dolbyio::comms::event_handler_id) { BroadcastEvent(OnInitialized); })
	    .on_error(DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::UpdateStatus(dolbyio::comms::conference_status Status)
{
	ConferenceStatus = Status;
	UE_LOG(LogDolbyIO, Log, TEXT("Conference status: %s"), *ToString(ConferenceStatus));

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
		UE_LOG(LogDolbyIO, Warning, TEXT("Cannot connect - conference name cannot be empty"));
		return;
	}

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
	        [this](dolbyio::comms::conference_info&&)
	        {
		        SetSpatialEnvironment();
		        ToggleInputMute();
		        ToggleOutputMute();
	        })
	    .on_error(DLB_ERROR_HANDLER);
}

bool UDolbyIOSubsystem::CanConnect() const
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

bool UDolbyIOSubsystem::IsConnected() const
{
	return ConferenceStatus == dolbyio::comms::conference_status::joined;
}

void UDolbyIOSubsystem::SetSpatialEnvironment()
{
	// The SDK spatial settings expect meters as the default unit of length.
	// Unreal uses centimeters for scale, so the plugin's scale of "1" is a scale of "100" for the SDK.
	const float SdkScale = SpatialEnvironmentScale * 100;
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

void UDolbyIOSubsystem::DemoConference()
{
	if (!CanConnect())
	{
		return;
	}

	Sdk->session()
	    .open({})
	    .then(
	        [this](dolbyio::comms::services::session::user_info&& User)
	        {
		        LocalParticipantID = User.participant_id.value_or("").c_str();
		        return Sdk->conference().demo(dolbyio::comms::spatial_audio_style::shared);
	        })
	    .then(
	        [this](dolbyio::comms::conference_info&&)
	        {
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

	Sdk->conference().leave().then([this]() { return Sdk->session().close(); }).on_error(DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::SetSpatialEnvironmentScale(float Scale)
{
	SpatialEnvironmentScale = Scale;
	if (IsConnected())
	{
		SetSpatialEnvironment();
	}
}

void UDolbyIOSubsystem::MuteInput()
{
	bIsInputMuted = true;
	ToggleInputMute();
}

void UDolbyIOSubsystem::UnmuteInput()
{
	bIsInputMuted = false;
	ToggleInputMute();
}

void UDolbyIOSubsystem::MuteOutput()
{
	bIsOutputMuted = true;
	ToggleOutputMute();
}

void UDolbyIOSubsystem::UnmuteOutput()
{
	bIsOutputMuted = false;
	ToggleOutputMute();
}

void UDolbyIOSubsystem::GetAudioLevels()
{
	if (!IsConnected())
	{
		return;
	}

	Sdk->conference()
	    .get_all_audio_levels()
	    .then(
	        [this](std::vector<dolbyio::comms::audio_level>&& Levels)
	        {
		        TArray<FString> ActiveSpeakers;
		        TArray<float> AudioLevels;
		        for (const dolbyio::comms::audio_level& Level : Levels)
		        {
			        ActiveSpeakers.Add(Level.participant_id.c_str());
			        AudioLevels.Add(Level.level);
		        }
		        BroadcastEvent(OnAudioLevelsChanged, ActiveSpeakers, AudioLevels);
	        })
	    .on_error(DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::SetLocalPlayerLocation(const FVector& Location)
{
	if (LocationTimerHandle.IsValid())
	{
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
			SetLocalPlayerLocationImpl(FirstPlayerController->GetPawn()->GetActorLocation());
		}
	}
}

void UDolbyIOSubsystem::SetRotationUsingFirstPlayer()
{
	if (UWorld* World = GetGameInstance()->GetWorld())
	{
		if (APlayerController* FirstPlayerController = World->GetFirstPlayerController())
		{
			SetLocalPlayerRotationImpl(FirstPlayerController->GetPawn()->GetActorRotation());
		}
	}
}

template <class TDelegate, class... TArgs> void UDolbyIOSubsystem::BroadcastEvent(TDelegate& Event, TArgs&&... Args)
{
	if (bIsAlive)
	{
		AsyncTask(ENamedThreads::GameThread, [=] { Event.Broadcast(Args...); });
	}
}
