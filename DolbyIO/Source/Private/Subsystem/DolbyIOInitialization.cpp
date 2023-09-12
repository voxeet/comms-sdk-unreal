// Copyright 2023 Dolby Laboratories

#include "DolbyIO.h"

#include "DolbyIODevices.h"
#include "Utils/DolbyIOBroadcastEvent.h"
#include "Utils/DolbyIOConversions.h"
#include "Utils/DolbyIOErrorHandler.h"
#include "Utils/DolbyIOLogging.h"
#include "Video/DolbyIOVideoFrameHandler.h"
#include "Video/DolbyIOVideoSink.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/EngineVersion.h"
#include "Misc/Paths.h"
#include "TimerManager.h"

using namespace dolbyio::comms;
using namespace dolbyio::comms::plugin;
using namespace DolbyIO;

void UDolbyIOSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	ConferenceStatus = conference_status::destroyed;

	{
		FScopeLock Lock{&VideoSinksLock};
		VideoSinks.Emplace(LocalCameraTrackID, std::make_shared<FVideoSink>(LocalCameraTrackID));
		VideoSinks.Emplace(LocalScreenshareTrackID, std::make_shared<FVideoSink>(LocalScreenshareTrackID));
		LocalCameraFrameHandler = std::make_shared<FVideoFrameHandler>(VideoSinks[LocalCameraTrackID]);
		LocalScreenshareFrameHandler = std::make_shared<FVideoFrameHandler>(VideoSinks[LocalScreenshareTrackID]);
	}

	FTimerManager& TimerManager = GetGameInstance()->GetTimerManager();
	TimerManager.SetTimer(LocationTimerHandle, this, &UDolbyIOSubsystem::SetLocationUsingFirstPlayer, 0.1, true);
	TimerManager.SetTimer(RotationTimerHandle, this, &UDolbyIOSubsystem::SetRotationUsingFirstPlayer, 0.01, true);

	BroadcastEvent(OnTokenNeeded);
}

void UDolbyIOSubsystem::Deinitialize()
{
	DLB_UE_LOG("Deinitializing");

	FScopeLock Lock{&VideoSinksLock};
	for (auto& Sink : VideoSinks)
	{
		Sink.Value->Disable(); // ignore new frames now on
	}

	Super::Deinitialize();
}

namespace
{
	class FSdkLogCallback : public logger_sink_callback
	{
	public:
		void log(log_level Level, std::string_view Message) const override
		{
#define DLB_UE_LOG_SDK_BASE(Verbosity)                                           \
	DLB_UE_LOG_BASE(Verbosity, "%s", *ToFText(std::string{Message}).ToString()); \
	return;
			switch (Level)
			{
				case log_level::ERROR:
					DLB_UE_LOG_SDK_BASE(Error);
				case log_level::WARNING:
					DLB_UE_LOG_SDK_BASE(Warning);
				default:
					DLB_UE_LOG_SDK_BASE(Log);
			}
		}
	};
}

void UDolbyIOSubsystem::SetLogSettings(EDolbyIOLogLevel SdkLogLevel, EDolbyIOLogLevel MediaLogLevel,
                                       EDolbyIOLogLevel DvcLogLevel, bool bLogToConsole, bool bLogToFile)
{
	sdk::log_settings LogSettings;
	LogSettings.sdk_log_level = ToSdkLogLevel(SdkLogLevel);
	LogSettings.media_log_level = ToSdkLogLevel(MediaLogLevel);
	LogSettings.dvc_log_level = ToSdkLogLevel(DvcLogLevel);
	LogSettings.suppress_stdout_logs = true;

	if (bLogToConsole)
	{
		LogSettings.log_callback = std::make_shared<FSdkLogCallback>();
	}
	if (bLogToFile)
	{
		const FString& LogDir = FPaths::ProjectLogDir();
		DLB_UE_LOG("Logs will be saved in directory %s", *LogDir);
		LogSettings.log_directory = ToStdString(LogDir);
	}

	try
	{
		sdk::set_log_settings(LogSettings);
	}
	catch (...)
	{
		DLB_ERROR_HANDLER(OnSetLogSettingsError).HandleError();
	}
}

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
			DLB_ERROR_HANDLER(OnSetTokenError).HandleError();
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
		DLB_ERROR_HANDLER(OnSetTokenError).HandleError();
		return;
	}

	Devices = MakeShared<FDevices>(*this, Sdk->device_management());

#define DLB_REGISTER_HANDLER(Service, Event) \
	[this](event_handler_id) { return Sdk->Service().add_event_handler([this](const Event& Event) { Handle(Event); }); }

	const FString ComponentName = "unreal-sdk";
	const FString ComponentVersion = *IPluginManager::Get().FindPlugin("DolbyIO")->GetDescriptor().VersionName +
	                                 FString{"_UE"} + FEngineVersion::Current().ToString(EVersionComponent::Minor);
	DLB_UE_LOG("Registering component %s %s", *ComponentName, *ComponentVersion);
	Sdk->register_component_version(ToStdString(ComponentName), ToStdString(ComponentVersion))
	    .then(
	        [this](sdk::component_data)
	        {
		        return Sdk->conference().add_event_handler([this](const conference_status_updated& Event)
		                                                   { UpdateStatus(Event.status); });
	        })
	    .then(DLB_REGISTER_HANDLER(conference, active_speaker_changed))
	    .then(DLB_REGISTER_HANDLER(device_management, audio_device_changed))
	    .then(DLB_REGISTER_HANDLER(conference, audio_levels))
	    .then(DLB_REGISTER_HANDLER(conference, conference_message_received))
	    .then(DLB_REGISTER_HANDLER(conference, local_participant_updated))
	    .then(DLB_REGISTER_HANDLER(conference, remote_participant_added))
	    .then(DLB_REGISTER_HANDLER(conference, remote_participant_updated))
	    .then(DLB_REGISTER_HANDLER(conference, remote_video_track_added))
	    .then(DLB_REGISTER_HANDLER(conference, remote_video_track_removed))
	    .then(DLB_REGISTER_HANDLER(device_management, screen_share_error))
	    .then([this](event_handler_id)
#if PLATFORM_WINDOWS
	          { return Sdk->device_management().set_default_audio_device_policy(default_audio_device_policy::output); })
	    .then([this]
#endif
#if PLATFORM_WINDOWS | PLATFORM_MAC
	          // keep this comment here to avoid wrong clang-formatting
	          { return video_processor::create(*Sdk); })
	    .then([this](std::shared_ptr<video_processor> VideoProcessorPtr)
	          { VideoProcessor = std::move(VideoProcessorPtr); })
	    .then(
	        [this]
#endif
	        {
		        utils::vfs_event::add_event_handler(*Sdk, [this](const utils::vfs_event& Event) { Handle(Event); });

		        DLB_UE_LOG("Initialized");
		        BroadcastEvent(OnInitialized);
	        })
	    .on_error(DLB_ERROR_HANDLER(OnSetTokenError));
}

void UDolbyIOObserver::InitializeComponent()
{
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UDolbyIOSubsystem* DolbyIOSubsystem = GameInstance->GetSubsystem<UDolbyIOSubsystem>())
			{
#define DLB_BIND(Event) DolbyIOSubsystem->Event.AddDynamic(this, &UDolbyIOObserver::Fwd##Event);
				DLB_BIND(OnTokenNeeded);

				DLB_BIND(OnInitialized);
				DLB_BIND(OnSetTokenError);

				DLB_BIND(OnConnected);
				DLB_BIND(OnConnectError);
				DLB_BIND(OnDemoConferenceError);

				DLB_BIND(OnDisconnected);
				DLB_BIND(OnDisconnectError);

				DLB_BIND(OnSetSpatialEnvironmentScaleError);

				DLB_BIND(OnMuteInputError);

				DLB_BIND(OnUnmuteInputError);

				DLB_BIND(OnMuteOutputError);

				DLB_BIND(OnUnmuteOutputError);

				DLB_BIND(OnMuteParticipantError);

				DLB_BIND(OnUnmuteParticipantError);

				DLB_BIND(OnParticipantAdded);
				DLB_BIND(OnParticipantUpdated);
				DLB_BIND(OnRemoteParticipantConnected);
				DLB_BIND(OnRemoteParticipantDisconnected);

				DLB_BIND(OnLocalParticipantUpdated);

				DLB_BIND(OnVideoTrackAdded);

				DLB_BIND(OnVideoTrackRemoved);

				DLB_BIND(OnVideoTrackEnabled);

				DLB_BIND(OnVideoTrackDisabled);

				DLB_BIND(OnVideoEnabled);
				DLB_BIND(OnEnableVideoError);

				DLB_BIND(OnVideoDisabled);
				DLB_BIND(OnDisableVideoError);

				DLB_BIND(OnScreenshareSourcesReceived);
				DLB_BIND(OnGetScreenshareSourcesError);

				DLB_BIND(OnScreenshareStarted);
				DLB_BIND(OnStartScreenshareError);

				DLB_BIND(OnScreenshareStopped);
				DLB_BIND(OnStopScreenshareError);

				DLB_BIND(OnChangeScreenshareParametersError);

				DLB_BIND(OnCurrentScreenshareSourceReceived);
				DLB_BIND(OnGetCurrentScreenshareSourceError);

				DLB_BIND(OnActiveSpeakersChanged);

				DLB_BIND(OnAudioLevelsChanged);

				DLB_BIND(OnSetLocalPlayerLocationError);

				DLB_BIND(OnSetLocalPlayerRotationError);

				DLB_BIND(OnSetRemotePlayerLocationError);

				DLB_BIND(OnSetLogSettingsError);

				DLB_BIND(OnAudioInputDevicesReceived);
				DLB_BIND(OnGetAudioInputDevicesError);

				DLB_BIND(OnAudioOutputDevicesReceived);
				DLB_BIND(OnGetAudioOutputDevicesError);

				DLB_BIND(OnCurrentAudioInputDeviceReceived);
				DLB_BIND(OnGetCurrentAudioInputDeviceError);

				DLB_BIND(OnCurrentAudioOutputDeviceReceived);
				DLB_BIND(OnGetCurrentAudioOutputDeviceError);

				DLB_BIND(OnVideoDevicesReceived);
				DLB_BIND(OnGetVideoDevicesError);

				DLB_BIND(OnCurrentVideoDeviceReceived);
				DLB_BIND(OnGetCurrentVideoDeviceError);

				DLB_BIND(OnCurrentAudioInputDeviceChanged);
				DLB_BIND(OnSetAudioInputDeviceError);

				DLB_BIND(OnCurrentAudioOutputDeviceChanged);
				DLB_BIND(OnSetAudioOutputDeviceError);

				DLB_BIND(OnUpdateUserMetadataError);

				DLB_BIND(OnSetAudioCaptureModeError);

				DLB_BIND(OnSendMessageError);

				DLB_BIND(OnMessageReceived);

				FwdOnTokenNeeded();
			}
		}
	}
}
