// Copyright 2023 Dolby Laboratories

#include "DolbyIOSubsystem.h"

#include "DolbyIOConversions.h"
#include "DolbyIOLogging.h"
#include "DolbyIOVideoFrameHandler.h"
#include "DolbyIOVideoSink.h"

#include "Async/Async.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Misc/Paths.h"
#include "TimerManager.h"

using namespace dolbyio::comms;
using namespace DolbyIO;

constexpr auto LocalCameraTrackID = "local-camera";
constexpr auto LocalScreenshareTrackID = "local-screenshare";
constexpr bool bIsDeviceNone = true;

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

	VideoSinks.Emplace(LocalCameraTrackID, std::make_shared<FVideoSink>(LocalCameraTrackID));
	VideoSinks.Emplace(LocalScreenshareTrackID, std::make_shared<FVideoSink>(LocalScreenshareTrackID));
	LocalCameraFrameHandler = std::make_shared<FVideoFrameHandler>(VideoSinks[LocalCameraTrackID]);
	LocalScreenshareFrameHandler = std::make_shared<FVideoFrameHandler>(VideoSinks[LocalScreenshareTrackID]);

	FTimerManager& TimerManager = GetGameInstance()->GetTimerManager();
	TimerManager.SetTimer(LocationTimerHandle, this, &UDolbyIOSubsystem::SetLocationUsingFirstPlayer, 0.1, true);
	TimerManager.SetTimer(RotationTimerHandle, this, &UDolbyIOSubsystem::SetRotationUsingFirstPlayer, 0.01, true);

	OnTokenNeeded.Broadcast();
}

void UDolbyIOSubsystem::Deinitialize()
{
#if WITH_EDITOR && PLATFORM_WINDOWS
	// on Windows, closing the game while sharing an Unreal Editor window results in a deadlock if SDK is destroyed on
	// main thread
	AsyncTask(ENamedThreads::ActualRenderingThread, [MovedSdk = MoveTemp(Sdk)] {});
#else
	Sdk.Reset(); // make sure Sdk is dead so it doesn't call handle_frame on VideoSink during game destruction
#endif
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
		RefreshTokenCb.Reset(); // RefreshToken callback can be called only once
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

	Sdk->register_component_version("unreal-sdk", "1.1.0-beta.9")
	    .then(
	        [this](sdk::component_data)
	        {
		        return Sdk->conference().add_event_handler([this](const conference_status_updated& Event)
		                                                   { UpdateStatus(Event.status); });
	        })
	    .then(
	        [this](event_handler_id)
	        {
		        return Sdk->conference().add_event_handler(
		            [this](const remote_participant_added& Event)
		            {
			            if (!Event.participant.status)
			            {
				            return;
			            }
			            const FDolbyIOParticipantInfo Info = ToFDolbyIOParticipantInfo(Event.participant);
			            DLB_UE_LOG("Participant status added: UserID=%s Name=%s ExternalID=%s Status=%s", *Info.UserID,
			                       *Info.Name, *Info.ExternalID, *ToString(*Event.participant.status));

			            FScopeLock Lock{&RemoteParticipantsLock};
			            RemoteParticipants.Emplace(Info.UserID, Info);

			            BroadcastEvent(OnParticipantAdded, Info.Status, Info);

			            if (TArray<FDolbyIOVideoTrack>* ParticipantTracks = BufferedVideoTracks.Find(Info.UserID))
			            {
				            for (const FDolbyIOVideoTrack& VideoTrack : *ParticipantTracks)
				            {
					            DLB_UE_LOG("Video track added: TrackID=%s ParticipantID=%s", *VideoTrack.TrackID,
					                       *VideoTrack.ParticipantID);
					            BroadcastEvent(OnVideoTrackAdded, VideoTrack);
				            }
				            BufferedVideoTracks.Remove(Info.UserID);
			            }
		            });
	        })
	    .then(
	        [this](event_handler_id)
	        {
		        return Sdk->conference().add_event_handler(
		            [this](const remote_participant_updated& Event)
		            {
			            if (!Event.participant.status)
			            {
				            return;
			            }
			            const FDolbyIOParticipantInfo Info = ToFDolbyIOParticipantInfo(Event.participant);
			            DLB_UE_LOG("Participant status updated: UserID=%s Name=%s ExternalID=%s Status=%s",
			                       *Info.UserID, *Info.Name, *Info.ExternalID, *ToString(*Event.participant.status));
			            {
				            FScopeLock Lock{&RemoteParticipantsLock};
				            RemoteParticipants.FindOrAdd(Info.UserID) = Info;
			            }

			            BroadcastEvent(OnParticipantUpdated, Info.Status, Info);
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
		            [this](const remote_video_track_added& Event)
		            {
			            const FDolbyIOVideoTrack VideoTrack = ToFDolbyIOVideoTrack(Event.track);

			            VideoSinks.Emplace(VideoTrack.TrackID, std::make_shared<FVideoSink>(VideoTrack.TrackID));
			            Sdk->video()
			                .remote()
			                .set_video_sink(Event.track, VideoSinks[VideoTrack.TrackID])
			                .on_error(MAKE_DLB_ERROR_HANDLER);

			            FScopeLock Lock{&RemoteParticipantsLock};
			            if (RemoteParticipants.Contains(VideoTrack.ParticipantID))
			            {
				            DLB_UE_LOG("Video track added: TrackID=%s ParticipantID=%s", *VideoTrack.TrackID,
				                       *VideoTrack.ParticipantID);
				            BroadcastEvent(OnVideoTrackAdded, VideoTrack);
			            }
			            else
			            {
				            BufferedVideoTracks.FindOrAdd(VideoTrack.ParticipantID).Add(VideoTrack);
			            }
		            });
	        })
	    .then(
	        [this](event_handler_id)
	        {
		        return Sdk->conference().add_event_handler(
		            [this](const remote_video_track_removed& Event)
		            {
			            const FDolbyIOVideoTrack VideoTrack = ToFDolbyIOVideoTrack(Event.track);
			            DLB_UE_LOG("Video track removed: TrackID=%s ParticipantID=%s", *VideoTrack.TrackID,
			                       *VideoTrack.ParticipantID);

			            VideoSinks[VideoTrack.TrackID]->UnbindAllMaterials();
			            VideoSinks.Remove(VideoTrack.TrackID);
			            BroadcastEvent(OnVideoTrackRemoved, VideoTrack);
		            });
	        })
	    .then(
	        [this](event_handler_id)
	        {
		        return Sdk->device_management().add_event_handler(
		            [this](const audio_device_changed& Event)
		            {
			            if (!Event.device)
			            {
				            DLB_UE_LOG("Audio device changed for direction: %s to no device",
				                       *ToString(Event.utilized_direction));
				            if (Event.utilized_direction == audio_device::direction::input)
					            BroadcastEvent(OnCurrentAudioInputDeviceChanged, bIsDeviceNone, FDolbyIOAudioDevice{});
				            else
					            BroadcastEvent(OnCurrentAudioOutputDeviceChanged, bIsDeviceNone, FDolbyIOAudioDevice{});
				            return;
			            }
			            Sdk->device_management()
			                .get_audio_devices()
			                .then(
			                    [this, Event](const std::vector<audio_device>& DvcDevices)
			                    {
				                    for (const audio_device& Device : DvcDevices)
					                    if (*Event.device == Device.get_identity())
					                    {
						                    DLB_UE_LOG("Audio device changed for direction: %s to device - %s",
						                               *ToString(Event.utilized_direction), *ToString(Device));
						                    if (Event.utilized_direction == audio_device::direction::input)
							                    BroadcastEvent(OnCurrentAudioInputDeviceChanged, !bIsDeviceNone,
							                                   ToFDolbyIOAudioDevice(Device));
						                    else
							                    BroadcastEvent(OnCurrentAudioOutputDeviceChanged, !bIsDeviceNone,
							                                   ToFDolbyIOAudioDevice(Device));
						                    return;
					                    }
			                    })
			                .on_error(MAKE_DLB_ERROR_HANDLER);
		            });
	        })
	    .then([this](event_handler_id)
#if PLATFORM_WINDOWS
	          { return Sdk->device_management().set_default_audio_device_policy(default_audio_device_policy::output); })
	    .then(
	        [this]
#endif
	        {
		        using namespace dolbyio::comms::utils;
		        vfs_event::add_event_handler(*Sdk,
		                                     [this](const vfs_event& Event)
		                                     {
			                                     for (const auto& TrackMapItem : Event.new_enabled)
			                                     {
				                                     const FDolbyIOVideoTrack VideoTrack =
				                                         ToFDolbyIOVideoTrack(TrackMapItem);
				                                     DLB_UE_LOG("Video track ID %s for participant ID %s enabled",
				                                                *VideoTrack.TrackID, *VideoTrack.ParticipantID);
				                                     BroadcastEvent(OnVideoTrackEnabled, VideoTrack);
			                                     }
			                                     for (const auto& TrackMapItem : Event.new_disabled)
			                                     {
				                                     const FDolbyIOVideoTrack VideoTrack =
				                                         ToFDolbyIOVideoTrack(TrackMapItem);
				                                     DLB_UE_LOG("Video track ID %s for participant ID %s disabled",
				                                                *VideoTrack.TrackID, *VideoTrack.ParticipantID);
				                                     BroadcastEvent(OnVideoTrackDisabled, VideoTrack);
			                                     }
		                                     });

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
			BroadcastEvent(OnConnected, LocalParticipantID, ConferenceID);
			break;
		case conference_status::left:
		case conference_status::error:
			BroadcastEvent(OnDisconnected);
			EmptyRemoteParticipants();
			break;
	}
}

void UDolbyIOSubsystem::Connect(const FString& ConferenceName, const FString& UserName, const FString& ExternalID,
                                const FString& AvatarURL, EDolbyIOConnectionMode ConnMode,
                                EDolbyIOSpatialAudioStyle SpatialStyle, int MaxVideoStreams,
                                EDolbyIOVideoForwardingStrategy VideoForwardingStrategy)
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
	DLB_UE_LOG("Connecting to conference %s with user name \"%s\" (%s, %s)", *ConferenceName, *UserName,
	           *UEnum::GetValueAsString(ConnectionMode), *UEnum::GetValueAsString(SpatialAudioStyle));

	services::session::user_info UserInfo{};
	UserInfo.name = ToStdString(UserName);
	UserInfo.externalId = ToStdString(ExternalID);
	UserInfo.avatarUrl = ToStdString(AvatarURL);
	EmptyRemoteParticipants();

	Sdk->session()
	    .open(MoveTemp(UserInfo))
	    .then(
	        [this, ConferenceName = ToStdString(ConferenceName)](services::session::user_info&& User)
	        {
		        LocalParticipantID = ToFString(User.participant_id.value_or(""));

		        conference::conference_options Options{};
		        Options.alias = ConferenceName;
		        Options.params.spatial_audio_style = ToSdkSpatialAudioStyle(SpatialAudioStyle);
		        return Sdk->conference().create(Options);
	        })
	    .then(
	        [this, MaxVideoStreams, VideoForwardingStrategy](conference_info&& ConferenceInfo)
	        {
		        ConferenceID = ToFString(ConferenceInfo.id);
		        if (ConnectionMode == EDolbyIOConnectionMode::Active)
		        {
			        conference::join_options Options{};
			        Options.constraints.audio = true;
			        Options.constraints.video = bIsVideoEnabled;
			        Options.connection.spatial_audio = IsSpatialAudio();
			        Options.connection.max_video_forwarding = MaxVideoStreams;
			        Options.connection.forwarding_strategy =
			            VideoForwardingStrategy == EDolbyIOVideoForwardingStrategy::LastSpeaker
			                ? video_forwarding_strategy::last_speaker
			                : video_forwarding_strategy::closest_user;
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
		        DLB_UE_LOG("Connected to conference ID %s with user ID %s", *ConferenceID, *LocalParticipantID);
		        SetSpatialEnvironment();
		        ToggleInputMute();
		        ToggleOutputMute();
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

void UDolbyIOSubsystem::EmptyRemoteParticipants()
{
	FScopeLock Lock{&RemoteParticipantsLock};
	RemoteParticipants.Empty();
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

void UDolbyIOSubsystem::DemoConference()
{
	if (!CanConnect())
	{
		return;
	}

	DLB_UE_LOG("Connecting to demo conference");
	ConnectionMode = EDolbyIOConnectionMode::Active;
	SpatialAudioStyle = EDolbyIOSpatialAudioStyle::Shared;
	EmptyRemoteParticipants();

	Sdk->session()
	    .open({})
	    .then(
	        [this](services::session::user_info&& User)
	        {
		        LocalParticipantID = ToFString(User.participant_id.value_or(""));
		        return Sdk->conference().demo(ToSdkSpatialAudioStyle(SpatialAudioStyle));
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

void UDolbyIOSubsystem::MuteParticipant(const FString& ParticipantID)
{
	if (!IsConnected() || ParticipantID == LocalParticipantID)
	{
		return;
	}

	DLB_UE_LOG("Muting participant ID %s", *ParticipantID);
	Sdk->audio().remote().stop(ToStdString(ParticipantID)).on_error(MAKE_DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::UnmuteParticipant(const FString& ParticipantID)
{
	if (!IsConnected() || ParticipantID == LocalParticipantID)
	{
		return;
	}

	DLB_UE_LOG("Unmuting participant ID %s", *ParticipantID);
	Sdk->audio().remote().start(ToStdString(ParticipantID)).on_error(MAKE_DLB_ERROR_HANDLER);
}

TArray<FDolbyIOParticipantInfo> UDolbyIOSubsystem::GetParticipants()
{
	TArray<FDolbyIOParticipantInfo> Ret;
	{
		FScopeLock Lock{&RemoteParticipantsLock};
		RemoteParticipants.GenerateValueArray(Ret);
	}
	return Ret;
}

void UDolbyIOSubsystem::EnableVideo(const FDolbyIOVideoDevice& VideoDevice)
{
	if (!Sdk)
	{
		DLB_UE_WARN("Cannot enable video - not initialized");
		return;
	}

	DLB_UE_LOG("Enabling video");
	static const camera_device DefaultCamera{};
	Sdk->video()
	    .local()
	    .start(ToSdkVideoDevice(VideoDevice), LocalCameraFrameHandler)
	    .then(
	        [this]
	        {
		        bIsVideoEnabled = true;
		        OnVideoEnabled.Broadcast(LocalCameraTrackID);
	        })
	    .on_error(MAKE_DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::DisableVideo()
{
	if (!Sdk)
	{
		return;
	}

	DLB_UE_LOG("Disabling video");
	Sdk->video()
	    .local()
	    .stop()
	    .then(
	        [this]
	        {
		        bIsVideoEnabled = false;
		        OnVideoDisabled.Broadcast(LocalCameraTrackID);
	        })
	    .on_error(MAKE_DLB_ERROR_HANDLER);
}

UTexture2D* UDolbyIOSubsystem::GetTexture(const FString& VideoTrackID)
{
	if (const std::shared_ptr<FVideoSink>* Sink = VideoSinks.Find(VideoTrackID))
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
                                         EDolbyIOScreenshareEncoderHint EncoderHint,
                                         EDolbyIOScreenshareMaxResolution MaxResolution,
                                         EDolbyIOScreenshareDownscaleQuality DownscaleQuality)
{
	if (!IsConnectedAsActive())
	{
		DLB_UE_WARN("Cannot start screenshare - not connected as active user");
		return;
	}

	DLB_UE_LOG("Starting screenshare using source: ID=%d IsScreen=%d Title=%s %s %s %s", Source.ID, Source.bIsScreen,
	           *Source.Title.ToString(), *UEnum::GetValueAsString(EncoderHint), *UEnum::GetValueAsString(MaxResolution),
	           *UEnum::GetValueAsString(DownscaleQuality));
	Sdk->conference()
	    .start_screen_share(screen_share_source{ToStdString(Source.Title.ToString()), Source.ID,
	                                            Source.bIsScreen ? screen_share_source::type::screen
	                                                             : screen_share_source::type::window},
	                        LocalScreenshareFrameHandler,
	                        ToSdkContentInfo(EncoderHint, MaxResolution, DownscaleQuality))
	    .then([this] { OnScreenshareStarted.Broadcast(LocalScreenshareTrackID); })
	    .on_error(MAKE_DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::StopScreenshare()
{
	if (!Sdk)
	{
		return;
	}

	DLB_UE_LOG("Stopping screenshare");
	Sdk->conference()
	    .stop_screen_share()
	    .then([this] { OnScreenshareStopped.Broadcast(LocalScreenshareTrackID); })
	    .on_error(MAKE_DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::ChangeScreenshareParameters(EDolbyIOScreenshareEncoderHint EncoderHint,
                                                    EDolbyIOScreenshareMaxResolution MaxResolution,
                                                    EDolbyIOScreenshareDownscaleQuality DownscaleQuality)
{
	if (!IsConnectedAsActive())
	{
		return;
	}
	DLB_UE_LOG("Changing screenshare parameters to %s %s %s", *UEnum::GetValueAsString(EncoderHint),
	           *UEnum::GetValueAsString(MaxResolution), *UEnum::GetValueAsString(DownscaleQuality));
	Sdk->conference()
	    .screen_share_content_info(ToSdkContentInfo(EncoderHint, MaxResolution, DownscaleQuality))
	    .on_error(MAKE_DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::BindMaterial(UMaterialInstanceDynamic* Material, const FString& VideoTrackID)
{
	for (auto& Sink : VideoSinks)
	{
		if (Sink.Key != VideoTrackID)
		{
			Sink.Value->UnbindMaterial(Material);
		}
	}

	if (const std::shared_ptr<DolbyIO::FVideoSink>* Sink = VideoSinks.Find(VideoTrackID))
	{
		(*Sink)->BindMaterial(Material);
	}
}

void UDolbyIOSubsystem::UnbindMaterial(UMaterialInstanceDynamic* Material, const FString& VideoTrackID)
{
	if (const std::shared_ptr<DolbyIO::FVideoSink>* Sink = VideoSinks.Find(VideoTrackID))
	{
		(*Sink)->UnbindMaterial(Material);
	}
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

void UDolbyIOSubsystem::SetRemotePlayerLocation(const FString& ParticipantID, const FVector& Location)
{
	if (!IsConnectedAsActive() || SpatialAudioStyle != EDolbyIOSpatialAudioStyle::Individual ||
	    ParticipantID == LocalParticipantID)
	{
		return;
	}

	Sdk->conference()
	    .set_spatial_position(ToStdString(ParticipantID), {Location.X, Location.Y, Location.Z})
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

void UDolbyIOSubsystem::SetLogSettings(EDolbyIOLogLevel SdkLogLevel, EDolbyIOLogLevel MediaLogLevel,
                                       EDolbyIOLogLevel DvcLogLevel)
{
	const FString& LogDir = FPaths::ProjectLogDir();
	DLB_UE_LOG("Logs will be saved in directory %s", *LogDir);

	sdk::log_settings LogSettings;
	LogSettings.sdk_log_level = ToSdkLogLevel(SdkLogLevel);
	LogSettings.media_log_level = ToSdkLogLevel(MediaLogLevel);
	LogSettings.dvc_log_level = ToSdkLogLevel(DvcLogLevel);
	LogSettings.log_directory = ToStdString(LogDir);
	LogSettings.suppress_stdout_logs = true;
	sdk::set_log_settings(LogSettings);
}

void UDolbyIOSubsystem::GetAudioInputDevices()
{
	if (!Sdk)
	{
		DLB_UE_WARN("Cannot get audio input devices - not initialized");
		return;
	}

	DLB_UE_LOG("Getting audio input devices");
	Sdk->device_management()
	    .get_audio_devices()
	    .then(
	        [this](const std::vector<audio_device>& DvcDevices)
	        {
		        TArray<FDolbyIOAudioDevice> Devices;
		        for (const audio_device& Device : DvcDevices)
		        {
			        if (Device.direction() & audio_device::direction::input)
			        {
				        DLB_UE_LOG("Got audio input device: %s", *ToString(Device));
				        Devices.Add(ToFDolbyIOAudioDevice(Device));
			        }
		        }
		        BroadcastEvent(OnAudioInputDevicesReceived, Devices);
	        })
	    .on_error(MAKE_DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::GetAudioOutputDevices()
{
	if (!Sdk)
	{
		DLB_UE_WARN("Cannot get audio output devices - not initialized");
		return;
	}

	DLB_UE_LOG("Getting audio output devices");
	Sdk->device_management()
	    .get_audio_devices()
	    .then(
	        [this](const std::vector<audio_device>& DvcDevices)
	        {
		        TArray<FDolbyIOAudioDevice> Devices;
		        for (const audio_device& Device : DvcDevices)
		        {
			        if (Device.direction() & audio_device::direction::output)
			        {
				        DLB_UE_LOG("Got audio output device: %s", *ToString(Device));
				        Devices.Add(ToFDolbyIOAudioDevice(Device));
			        }
		        }
		        BroadcastEvent(OnAudioOutputDevicesReceived, Devices);
	        })
	    .on_error(MAKE_DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::GetCurrentAudioInputDevice()
{
	if (!Sdk)
	{
		DLB_UE_WARN("Cannot get current audio input device - not initialized");
		return;
	}

	DLB_UE_LOG("Getting current audio input device");
	Sdk->device_management()
	    .get_current_audio_input_device()
	    .then(
	        [this](std::optional<audio_device> Device)
	        {
		        if (!Device)
		        {
			        DLB_UE_LOG("Got current audio input device - none");
			        BroadcastEvent(OnCurrentAudioInputDeviceReceived, bIsDeviceNone, FDolbyIOAudioDevice{});
			        return;
		        }
		        DLB_UE_LOG("Got current audio input device - %s", *ToString(*Device));
		        BroadcastEvent(OnCurrentAudioInputDeviceReceived, !bIsDeviceNone, ToFDolbyIOAudioDevice(*Device));
	        })
	    .on_error(MAKE_DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::GetCurrentAudioOutputDevice()
{
	if (!Sdk)
	{
		DLB_UE_WARN("Cannot get current audio output device - not initialized");
		return;
	}

	DLB_UE_LOG("Getting current audio output device");
	Sdk->device_management()
	    .get_current_audio_output_device()
	    .then(
	        [this](std::optional<audio_device> Device)
	        {
		        if (!Device)
		        {
			        DLB_UE_LOG("Got current audio output device - none");
			        BroadcastEvent(OnCurrentAudioOutputDeviceReceived, bIsDeviceNone, FDolbyIOAudioDevice{});
			        return;
		        }
		        DLB_UE_LOG("Got current audio output device - %s", *ToString(*Device));
		        BroadcastEvent(OnCurrentAudioOutputDeviceReceived, !bIsDeviceNone, ToFDolbyIOAudioDevice(*Device));
	        })
	    .on_error(MAKE_DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::SetAudioInputDevice(const FString& NativeID)
{
	if (!Sdk)
	{
		DLB_UE_WARN("Cannot set audio input device - not initialized");
		return;
	}

	DLB_UE_LOG("Setting audio input device with native ID %s", *NativeID);
	Sdk->device_management()
	    .get_audio_devices()
	    .then(
	        [this, SdkNativeID = ToSdkNativeDeviceID(NativeID)](const std::vector<audio_device>& DvcDevices)
	        {
		        for (const audio_device& Device : DvcDevices)
			        if (Device.direction() & audio_device::direction::input && Device.native_id() == SdkNativeID)
			        {
				        DLB_UE_LOG("Setting audio input device to %s", *ToString(Device));
				        Sdk->device_management().set_preferred_input_audio_device(Device).on_error(
				            MAKE_DLB_ERROR_HANDLER);
				        return;
			        }
	        })
	    .on_error(MAKE_DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::SetAudioOutputDevice(const FString& NativeID)
{
	if (!Sdk)
	{
		DLB_UE_WARN("Cannot set audio output device - not initialized");
		return;
	}

	DLB_UE_LOG("Setting audio output device with native ID %s", *NativeID);
	Sdk->device_management()
	    .get_audio_devices()
	    .then(
	        [this, SdkNativeID = ToSdkNativeDeviceID(NativeID)](const std::vector<audio_device>& DvcDevices)
	        {
		        for (const audio_device& Device : DvcDevices)
			        if (Device.direction() & audio_device::direction::output && Device.native_id() == SdkNativeID)
			        {
				        DLB_UE_LOG("Setting audio output device to %s", *ToString(Device));
				        Sdk->device_management().set_preferred_output_audio_device(Device).on_error(
				            MAKE_DLB_ERROR_HANDLER);
				        return;
			        }
	        })
	    .on_error(MAKE_DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::GetVideoDevices()
{
	if (!Sdk)
	{
		DLB_UE_WARN("Cannot get video devices - not initialized");
		return;
	}

	DLB_UE_LOG("Getting video devices");
	Sdk->device_management()
	    .get_video_devices()
	    .then(
	        [this](const std::vector<camera_device>& DvcDevices)
	        {
		        TArray<FDolbyIOVideoDevice> Devices;
		        Devices.Reserve(DvcDevices.size());
		        for (const camera_device& Device : DvcDevices)
		        {
			        DLB_UE_LOG("Got video device - display_name: %s unique_id: %s", *ToFString(Device.display_name),
			                   *ToFString(Device.unique_id));
			        Devices.Add(ToFDolbyIOVideoDevice(Device));
		        }
		        BroadcastEvent(OnVideoDevicesReceived, Devices);
	        })
	    .on_error(MAKE_DLB_ERROR_HANDLER);
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
