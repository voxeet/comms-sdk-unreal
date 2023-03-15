// Copyright 2023 Dolby Laboratories

#include "DolbyIOSubsystem.h"

#include "DolbyIOConversions.h"
#include "DolbyIOLogging.h"
#include "DolbyIOVideoSink.h"

#include "Async/Async.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

using namespace dolbyio::comms;
using namespace DolbyIO;

void UDolbyIOSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

#if PLATFORM_WINDOWS
	sdk::set_app_allocator(
	    {::operator new,
	     [](std::size_t Count, std::size_t Al) { return ::operator new(Count, static_cast<std::align_val_t>(Al)); },
	     ::operator delete,
	     [](void* Ptr, std::size_t Al) { ::operator delete(Ptr, static_cast<std::align_val_t>(Al)); }});
#endif

	ConferenceStatus = conference_status::destroyed;

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

#define MAKE_DLB_ERROR_HANDLER FErrorHandler(*this, __LINE__)

void UDolbyIOSubsystem::SetToken(const FString& Token)
{
	if (!Sdk)
	{
		DLB_UE_LOG("Initializing with token: %s", *Token);
		AsyncTask(ENamedThreads::AnyThread, [this, Token] { Initialize(Token); });
	}
	else if (RefreshTokenCb)
	{
		DLB_UE_LOG("Refreshing token: %s", *Token);
		try
		{
			(*RefreshTokenCb)(ToStdString(Token));
		}
		catch (...)
		{
			MAKE_DLB_ERROR_HANDLER.HandleError();
		}
		RefreshTokenCb.Reset(); // RefreshToken callback can only be called once
	}
}

void UDolbyIOSubsystem::Initialize(const FString& Token)
{
	try
	{
		Sdk = TSharedPtr<sdk>(sdk::create(ToStdString(Token),
		                                  [this](std::unique_ptr<refresh_token>&& RefreshCb)
		                                  {
			                                  DLB_UE_LOG("Refresh token requested");
			                                  RefreshTokenCb = TSharedPtr<refresh_token>(RefreshCb.release());
			                                  BroadcastEvent(OnTokenNeeded);
		                                  })
		                          .release());
	}
	catch (...)
	{
		MAKE_DLB_ERROR_HANDLER.HandleError();
		return;
	}

	Sdk->register_component_version("unreal_sdk", "1.1.0-beta.1")
	    .then(
	        [this]
	        {
		        return Sdk->conference().add_event_handler([this](const conference_status_updated& Event)
		                                                   { UpdateStatus(Event.status); });
	        })
	    .then(
	        [this](event_handler_id)
	        {
		        return Sdk->conference().add_event_handler(
		            [this](const participant_added& Event)
		            {
			            if (!Event.participant.status || Event.participant.user_id == ToStdString(LocalParticipantID))
			            {
				            return;
			            }
			            const FDolbyIOParticipantInfo Info = ToFDolbyIOParticipantInfo(Event.participant);
			            DLB_UE_LOG("Participant status added: UserID=%s Name=%s ExternalID=%s Status=%s", *Info.UserID,
			                       *Info.Name, *Info.ExternalID, *ToString(*Event.participant.status));

			            return BroadcastEvent(OnParticipantAdded, Info.Status, Info);
		            });
	        })
	    .then(
	        [this](event_handler_id)
	        {
		        return Sdk->conference().add_event_handler(
		            [this](const participant_updated& Event)
		            {
			            if (!Event.participant.status || Event.participant.user_id == ToStdString(LocalParticipantID))
			            {
				            return;
			            }
			            const FDolbyIOParticipantInfo Info = ToFDolbyIOParticipantInfo(Event.participant);
			            DLB_UE_LOG("Participant status updated: UserID=%s Name=%s ExternalID=%s Status=%s",
			                       *Info.UserID, *Info.Name, *Info.ExternalID, *ToString(*Event.participant.status));

			            return BroadcastEvent(OnParticipantUpdated, Info.Status, Info);
		            });
	        })
	    .then(
	        [this](event_handler_id)
	        {
		        return Sdk->conference().add_event_handler(
		            [this](const active_speaker_changed& Event)
		            {
			            TArray<FString> ActiveSpeakers;
			            for (const std::string& Speaker : Event.active_speakers)
			            {
				            ActiveSpeakers.Add(ToFString(Speaker));
			            }
			            BroadcastEvent(OnActiveSpeakersChanged, ActiveSpeakers);
		            });
	        })
	    .then(
	        [this](event_handler_id)
	        {
		        return Sdk->conference().add_event_handler(
		            [this](const audio_levels& Event)
		            {
			            TArray<FString> ActiveSpeakers;
			            TArray<float> AudioLevels;
			            for (const audio_level& Level : Event.levels)
			            {
				            ActiveSpeakers.Add(ToFString(Level.participant_id));
				            AudioLevels.Add(Level.level);
			            }
			            BroadcastEvent(OnAudioLevelsChanged, ActiveSpeakers, AudioLevels);
		            });
	        })
	    .then(
	        [this](event_handler_id)
	        {
		        return Sdk->conference().add_event_handler(
		            [this](const video_track_added& Event)
		            {
			            if (Event.track.remote)
			            {
				            const FString ParticipantID = ToFString(Event.track.peer_id);
				            const FString StreamID = ToFString(Event.track.stream_id);
				            DLB_UE_LOG("Video track added: ParticipantID=%s StreamID=%s", *ParticipantID, *StreamID);
				            VideoSinks.Emplace(ParticipantID, std::make_shared<FVideoSink>());
				            Sdk->video()
				                .remote()
				                .set_video_sink(Event.track, VideoSinks[ParticipantID])
				                .on_error(MAKE_DLB_ERROR_HANDLER);
				            BroadcastEvent(OnVideoTrackAdded, ParticipantID);
			            }
		            });
	        })
	    .then(
	        [this](event_handler_id)
	        {
		        return Sdk->conference().add_event_handler(
		            [this](const video_track_removed& Event)
		            {
			            if (Event.track.remote)
			            {
				            const FString ParticipantID = ToFString(Event.track.peer_id);
				            const FString StreamID = ToFString(Event.track.stream_id);
				            DLB_UE_LOG("Video track removed: ParticipantID=%s StreamID=%s", *ParticipantID, *StreamID);
				            VideoSinks.Remove(ParticipantID);
				            BroadcastEvent(OnVideoTrackRemoved, ParticipantID);
			            }
		            });
	        })
	    .then([this](event_handler_id)
#if PLATFORM_WINDOWS
	          { return Sdk->device_management().set_default_audio_device_policy(default_audio_device_policy::output); })
	    .then(
	        [this]
#endif
	        {
		        DLB_UE_LOG("Initialized");
		        BroadcastEvent(OnInitialized);
	        })
	    .on_error(MAKE_DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::UpdateStatus(conference_status Status)
{
	ConferenceStatus = Status;
	DLB_UE_LOG("Conference status: %s", *ToString(ConferenceStatus));

	switch (ConferenceStatus)
	{
		case conference_status::joined:
			BroadcastEvent(OnConnected, LocalParticipantID);
			break;
		case conference_status::left:
		case conference_status::error:
			BroadcastEvent(OnDisconnected);
			break;
	}
}

void UDolbyIOSubsystem::Connect(const FString& ConferenceName, const FString& UserName, const FString& ExternalID,
                                const FString& AvatarURL, EDolbyIOConnectionMode ConnMode,
                                EDolbyIOSpatialAudioStyle SpatialStyle)
{
	using namespace dolbyio::comms::services;

	if (!CanConnect())
	{
		return;
	}
	if (ConferenceName.IsEmpty())
	{
		DLB_UE_WARN("Cannot connect - conference name cannot be empty");
		return;
	}

	ConnectionMode = ConnMode;
	SpatialAudioStyle = SpatialStyle;
	DLB_UE_LOG("Connecting to conference %s as %s with name %s with %s spatial audio", *ConferenceName,
	           *ToString(ConnectionMode), *UserName, *ToString(SpatialAudioStyle));

	session::user_info UserInfo{};
	UserInfo.name = ToStdString(UserName);
	UserInfo.externalId = ToStdString(ExternalID);
	UserInfo.avatarUrl = ToStdString(AvatarURL);

	Sdk->session()
	    .open(MoveTemp(UserInfo))
	    .then(
	        [this, ConferenceName = ToStdString(ConferenceName)](session::user_info&& User)
	        {
		        LocalParticipantID = ToFString(User.participant_id.value_or(""));

		        conference::conference_options Options{};
		        Options.alias = ConferenceName;
		        Options.params.spatial_audio_style = ToSdkSpatialAudioStyle(SpatialAudioStyle);
		        return Sdk->conference().create(Options);
	        })
	    .then(
	        [this](conference_info&& ConferenceInfo)
	        {
		        if (ConnectionMode == EDolbyIOConnectionMode::Active)
		        {
			        conference::join_options Options{};
			        Options.constraints.audio = true;
			        Options.connection.spatial_audio = IsSpatialAudio();
			        return Sdk->conference().join(ConferenceInfo, Options);
		        }
		        else
		        {
			        conference::listen_options Options{};
			        Options.connection.spatial_audio = IsSpatialAudio();
			        Options.type = ConnectionMode == EDolbyIOConnectionMode::ListenerRegular ? listen_mode::regular
			                                                                                 : listen_mode::rts_mixed;
			        return Sdk->conference().listen(ConferenceInfo, Options);
		        }
	        })
	    .then(
	        [this](conference_info&& ConferenceInfo)
	        {
		        DLB_UE_LOG("Connected to conference ID %s with user ID %s", *ToFString(ConferenceInfo.id),
		                   *LocalParticipantID);
		        SetSpatialEnvironment();
		        ToggleInputMute();
		        ToggleOutputMute();

		        if (bIsVideoEnabled)
		        {
			        Sdk->video().local().start().on_error(MAKE_DLB_ERROR_HANDLER);
		        }
	        })
	    .on_error(MAKE_DLB_ERROR_HANDLER);
}

bool UDolbyIOSubsystem::CanConnect() const
{
	if (!Sdk)
	{
		DLB_UE_WARN("Cannot connect - not initialized");
		return false;
	}
	if (IsConnected())
	{
		DLB_UE_WARN("Cannot connect - already connected, please disconnect first");
		return false;
	}
	return true;
}

bool UDolbyIOSubsystem::IsConnected() const
{
	return ConferenceStatus == conference_status::joined;
}

bool UDolbyIOSubsystem::IsConnectedAsActive() const
{
	return IsConnected() && ConnectionMode == EDolbyIOConnectionMode::Active;
}

bool UDolbyIOSubsystem::IsSpatialAudio() const
{
	return SpatialAudioStyle != EDolbyIOSpatialAudioStyle::Disabled;
}

void UDolbyIOSubsystem::SetSpatialEnvironment()
{
	if (!IsConnectedAsActive() || !IsSpatialAudio())
	{
		return;
	}

	// The SDK spatial settings expect meters as the default unit of length.
	// Unreal uses centimeters for scale, so the plugin's scale of "1" is a scale of "100" for the SDK.
	const float SdkScale = SpatialEnvironmentScale * ScaleCenti;
	const spatial_scale Scale{SdkScale, SdkScale, SdkScale};
	const spatial_position Forward{1, 0, 0};
	const spatial_position Up{0, 0, 1};
	const spatial_position Right{0, 1, 0};
	Sdk->conference().set_spatial_environment(Scale, Forward, Up, Right).on_error(MAKE_DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::ToggleInputMute()
{
	if (IsConnectedAsActive())
	{
		Sdk->conference().mute(bIsInputMuted).on_error(MAKE_DLB_ERROR_HANDLER);
	}
}

void UDolbyIOSubsystem::ToggleOutputMute()
{
	if (IsConnected() && ConnectionMode != EDolbyIOConnectionMode::ListenerRTS)
	{
		Sdk->conference().mute_output(bIsOutputMuted).on_error(MAKE_DLB_ERROR_HANDLER);
	}
}

void UDolbyIOSubsystem::ToggleVideo()
{
	if (IsConnectedAsActive())
	{
		bIsVideoEnabled ? Sdk->video().local().start().on_error(MAKE_DLB_ERROR_HANDLER)
		                : Sdk->video().local().stop().on_error(MAKE_DLB_ERROR_HANDLER);
	}
}

void UDolbyIOSubsystem::DemoConference()
{
	if (!CanConnect())
	{
		return;
	}

	DLB_UE_LOG("Connecting to demo conference");

	Sdk->session()
	    .open({})
	    .then(
	        [this](services::session::user_info&& User)
	        {
		        LocalParticipantID = ToFString(User.participant_id.value_or(""));
		        return Sdk->conference().demo(spatial_audio_style::shared);
	        })
	    .then(
	        [this](conference_info&& ConferenceInfo)
	        {
		        DLB_UE_LOG("Connected to conference ID %s", *ToFString(ConferenceInfo.id));
		        SetSpatialEnvironment();
		        ToggleInputMute();
		        ToggleOutputMute();
	        })
	    .on_error(MAKE_DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::Disconnect()
{
	if (!IsConnected())
	{
		return;
	}

	DLB_UE_LOG("Disconnecting");
	Sdk->conference().leave().then([this]() { return Sdk->session().close(); }).on_error(MAKE_DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::SetSpatialEnvironmentScale(float Scale)
{
	DLB_UE_LOG("Setting spatial environment scale: %f", Scale);
	SpatialEnvironmentScale = Scale;
	SetSpatialEnvironment();
}

void UDolbyIOSubsystem::MuteInput()
{
	DLB_UE_LOG("Muting input");
	bIsInputMuted = true;
	ToggleInputMute();
}

void UDolbyIOSubsystem::UnmuteInput()
{
	DLB_UE_LOG("Unmuting input");
	bIsInputMuted = false;
	ToggleInputMute();
}

void UDolbyIOSubsystem::MuteOutput()
{
	DLB_UE_LOG("Muting output");
	bIsOutputMuted = true;
	ToggleOutputMute();
}

void UDolbyIOSubsystem::UnmuteOutput()
{
	DLB_UE_LOG("Unmuting output");
	bIsOutputMuted = false;
	ToggleOutputMute();
}

void UDolbyIOSubsystem::EnableVideo()
{
	DLB_UE_LOG("Enabling video");
	bIsVideoEnabled = true;
	ToggleVideo();
}

void UDolbyIOSubsystem::DisableVideo()
{
	DLB_UE_LOG("Disabling video");
	bIsVideoEnabled = false;
	ToggleVideo();
}

UTexture2D* UDolbyIOSubsystem::GetTexture(const FString& ParticipantID)
{
	if (const std::shared_ptr<FVideoSink>* Sink = VideoSinks.Find(ParticipantID))
	{
		return (*Sink)->GetTexture();
	}
	return nullptr;
}

void UDolbyIOSubsystem::GetScreenshareSources()
{
	if (!Sdk)
	{
		DLB_UE_WARN("Cannot get screenshare sources - not initialized");
		return;
	}

	DLB_UE_LOG("Getting screenshare sources");
	Sdk->device_management()
	    .get_screen_share_sources()
	    .then(
	        [this](const std::vector<screen_share_source>& ScreenShareSource)
	        {
		        TArray<FDolbyIOScreenshareSource> Sources;
		        for (const screen_share_source& Source : ScreenShareSource)
		        {
			        Sources.Add(FDolbyIOScreenshareSource{
			            Source.id, Source.type == screen_share_source::type::screen,
			            Source.title.empty() ? FText::FromString(FString{"Screen "} + FString::FromInt(Source.id + 1))
			                                 : ToFText(Source.title)});
		        }
		        BroadcastEvent(OnScreenshareSourcesReceived, Sources);
	        })
	    .on_error(MAKE_DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::StartScreenshare(const FDolbyIOScreenshareSource& Source,
                                         EDolbyIOScreenshareContentType ContentType)
{
	if (!IsConnectedAsActive())
	{
		DLB_UE_WARN("Cannot start screenshare - not connected as active user");
		return;
	}

	DLB_UE_LOG("Starting screenshare using source: ID=%d IsScreen=%d Title=%s ContentType=%d", Source.ID,
	           Source.bIsScreen, *Source.Title.ToString(), ContentType);
	constexpr auto NullFrameHandler = nullptr;
	Sdk->conference()
	    .start_screen_share(screen_share_source{ToStdString(Source.Title.ToString()), Source.ID,
	                                            Source.bIsScreen ? screen_share_source::type::screen
	                                                             : screen_share_source::type::window},
	                        NullFrameHandler, ToSdkContentType(ContentType))
	    .on_error(MAKE_DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::StopScreenshare()
{
	if (!Sdk)
	{
		return;
	}

	DLB_UE_LOG("Stopping screenshare");
	Sdk->conference().stop_screen_share().on_error(MAKE_DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::ChangeScreenshareContentType(EDolbyIOScreenshareContentType ContentType)
{
	if (!IsConnectedAsActive())
	{
		return;
	}

	DLB_UE_LOG("Changing screenshare content type to %d", ContentType);
	Sdk->conference().screen_share_content_type(ToSdkContentType(ContentType)).on_error(MAKE_DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::SetLocalPlayerLocation(const FVector& Location)
{
	if (LocationTimerHandle.IsValid())
	{
		DLB_UE_LOG("Disabling automatic location setting");
		GetGameInstance()->GetTimerManager().ClearTimer(LocationTimerHandle);
	}
	SetLocalPlayerLocationImpl(Location);
}

void UDolbyIOSubsystem::SetLocalPlayerLocationImpl(const FVector& Location)
{
	if (!IsConnectedAsActive() || !IsSpatialAudio())
	{
		return;
	}

	Sdk->conference()
	    .set_spatial_position(ToStdString(LocalParticipantID), {Location.X, Location.Y, Location.Z})
	    .on_error(MAKE_DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::SetLocalPlayerRotation(const FRotator& Rotation)
{
	if (RotationTimerHandle.IsValid())
	{
		DLB_UE_LOG("Disabling automatic rotation setting");
		GetGameInstance()->GetTimerManager().ClearTimer(RotationTimerHandle);
	}
	SetLocalPlayerRotationImpl(Rotation);
}

void UDolbyIOSubsystem::SetLocalPlayerRotationImpl(const FRotator& Rotation)
{
	if (!IsConnectedAsActive() || !IsSpatialAudio())
	{
		return;
	}

	// The SDK expects the direction values to mean rotations around the {x,y,z} axes as specified by the
	// environment. In Unreal, rotation around x is roll (because x is forward), y is pitch and z is yaw.
	Sdk->conference()
	    .set_spatial_direction({Rotation.Roll, Rotation.Pitch, Rotation.Yaw})
	    .on_error(MAKE_DLB_ERROR_HANDLER);
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

UDolbyIOSubsystem::FErrorHandler::FErrorHandler(UDolbyIOSubsystem& DolbyIOSubsystem, int Line)
    : DolbyIOSubsystem(DolbyIOSubsystem), Line(Line)
{
}

void UDolbyIOSubsystem::FErrorHandler::operator()(std::exception_ptr&& ExcPtr) const
{
	HandleError([ExcP = MoveTemp(ExcPtr)] { std::rethrow_exception(ExcP); });
}

void UDolbyIOSubsystem::FErrorHandler::HandleError() const
{
	HandleError([] { throw; });
}

void UDolbyIOSubsystem::FErrorHandler::HandleError(TFunction<void()> Callee) const
try
{
	Callee();
}
catch (const conference_state_exception& Ex)
{
	LogException("dolbyio::comms::conference_state_exception", Ex.what());
}
catch (const invalid_token_exception& Ex)
{
	LogException("dolbyio::comms::invalid_token_exception", Ex.what());
	DolbyIOSubsystem.Sdk.Reset();
}
catch (const dvc_error_exception& Ex)
{
	LogException("dolbyio::comms::dvc_error_exception", Ex.what());
}
catch (const peer_connection_failed_exception& Ex)
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

void UDolbyIOSubsystem::FErrorHandler::LogException(const FString& Type, const FString& What) const
{
	DLB_UE_ERROR("Caught %s: %s (conference status: %s, line: %d)", *Type, *What,
	             *ToString(DolbyIOSubsystem.ConferenceStatus), Line);
}
