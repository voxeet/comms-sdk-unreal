// Copyright 2023 Dolby Laboratories

#pragma once

#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "DolbyIOCppSdkFwd.h"
#include "DolbyIOTypes.h"

#include <memory>

#include "DolbyIO.generated.h"

// clang-format off
DECLARE_DYNAMIC_MULTICAST_DELEGATE
(FDolbyIOOnTokenNeededDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE
(FDolbyIOOnInitializedDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams
(FDolbyIOOnConnectedDelegate,
const FString&, LocalParticipantID,
const FString&, ConferenceID);

DECLARE_DYNAMIC_MULTICAST_DELEGATE
(FDolbyIOOnDisconnectedDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams
(FDolbyIOOnParticipantAddedDelegate,
const EDolbyIOParticipantStatus, Status,
const FDolbyIOParticipantInfo&, ParticipantInfo);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams
(FDolbyIOOnParticipantUpdatedDelegate,
const EDolbyIOParticipantStatus, Status,
const FDolbyIOParticipantInfo&, ParticipantInfo);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam
(FDolbyIOOnRemoteParticipantConnectedDelegate,
const FDolbyIOParticipantInfo&, ParticipantInfo);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
FDolbyIOOnRemoteParticipantDisconnectedDelegate,
const FDolbyIOParticipantInfo&, ParticipantInfo);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams
(FDolbyIOOnLocalParticipantUpdatedDelegate,
const EDolbyIOParticipantStatus, Status,
const FDolbyIOParticipantInfo&, ParticipantInfo);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam
(FDolbyIOOnVideoTrackAddedDelegate,
const FDolbyIOVideoTrack&, VideoTrack);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam
(FDolbyIOOnVideoTrackRemovedDelegate,
const FDolbyIOVideoTrack&, VideoTrack);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam
(FDolbyIOOnVideoTrackEnabledDelegate,
const FDolbyIOVideoTrack&, VideoTrack);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam
(FDolbyIOOnVideoTrackDisabledDelegate,
const FDolbyIOVideoTrack&, VideoTrack);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam
(FDolbyIOOnVideoEnabledDelegate,
const FString&, VideoTrackID);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam
(FDolbyIOOnVideoDisabledDelegate,
const FString&, VideoTrackID);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam
(FDolbyIOOnScreenshareStartedDelegate,
const FString&, VideoTrackID);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam
(FDolbyIOOnScreenshareStoppedDelegate,
const FString&, VideoTrackID);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam
(FDolbyIOOnActiveSpeakersChangedDelegate,
const TArray<FString>&, ActiveSpeakers);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams
(FDolbyIOOnAudioLevelsChangedDelegate,
const TArray<FString>&, ActiveSpeakers,
const TArray<float>&, AudioLevels);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam
(FDolbyIOOnScreenshareSourcesReceivedDelegate,
const TArray<FDolbyIOScreenshareSource>&, Sources);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams
(FDolbyIOOnCurrentScreenshareSourceReceivedDelegate,
bool, IsNone,
const FDolbyIOScreenshareSource&, OptionalSource);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam
(FDolbyIOOnAudioInputDevicesReceivedDelegate,
const TArray<FDolbyIOAudioDevice>&, Devices);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam
(FDolbyIOOnAudioOutputDevicesReceivedDelegate,
const TArray<FDolbyIOAudioDevice>&, Devices);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams
(FDolbyIOOnCurrentAudioInputDeviceReceivedDelegate,
bool, IsNone,
const FDolbyIOAudioDevice&, OptionalDevice);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams
(FDolbyIOOnCurrentAudioOutputDeviceReceivedDelegate,
bool, IsNone,
const FDolbyIOAudioDevice&, OptionalDevice);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam
(FDolbyIOOnVideoDevicesReceivedDelegate,
const TArray<FDolbyIOVideoDevice>&, Devices);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams
(FDolbyIOOnCurrentVideoDeviceReceivedDelegate,
bool, IsNone,
const FDolbyIOVideoDevice&, OptionalDevice);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams
(FDolbyIOOnCurrentAudioInputDeviceChangedDelegate,
bool, IsNone,
const FDolbyIOAudioDevice&, OptionalDevice);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams
(FDolbyIOOnCurrentAudioOutputDeviceChangedDelegate,
bool, IsNone,
const FDolbyIOAudioDevice&, OptionalDevice);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams
(FDolbyIOOnMessageReceivedDelegate,
const FString&, Message,
const FDolbyIOParticipantInfo&, ParticipantInfo);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam
(FDolbyIOOnErrorDelegate,
const FString&, ErrorMsg);
// clang-format on

namespace DolbyIO
{
	class FDevices;
	class FErrorHandler;
	class FVideoFrameHandler;
	class FVideoSink;
	class dummy_frame_dropper;
}

UCLASS(DisplayName = "Dolby.io Subsystem")
class DOLBYIO_API UDolbyIOSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	friend class DolbyIO::FErrorHandler;

public:
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetToken(const FString& Token);
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnInitializedDelegate OnInitialized;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnSetTokenError;

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void Connect(const FString& ConferenceName = "unreal", const FString& UserName = "", const FString& ExternalID = "",
	             const FString& AvatarURL = "", EDolbyIOConnectionMode ConnectionMode = EDolbyIOConnectionMode::Active,
	             EDolbyIOSpatialAudioStyle SpatialAudioStyle = EDolbyIOSpatialAudioStyle::Shared,
	             int MaxVideoStreams = 25,
	             EDolbyIOVideoForwardingStrategy VideoForwardingStrategy = EDolbyIOVideoForwardingStrategy::LastSpeaker,
	             EDolbyIOVideoCodec VideoCodec = EDolbyIOVideoCodec::H264);
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnConnectedDelegate OnConnected;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnConnectError;

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void DemoConference();
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnDemoConferenceError;

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void Disconnect();
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnDisconnectedDelegate OnDisconnected;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnDisconnectError;

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetSpatialEnvironmentScale(float SpatialEnvironmentScale = 1.0f);
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnSetSpatialEnvironmentScaleError;

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void MuteInput();
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnMuteInputError;

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void UnmuteInput();
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnUnmuteInputError;

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void MuteOutput();
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnMuteOutputError;

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void UnmuteOutput();
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnUnmuteOutputError;

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void MuteParticipant(const FString& ParticipantID);
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnMuteParticipantError;

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void UnmuteParticipant(const FString& ParticipantID);
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnUnmuteParticipantError;

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	TArray<FDolbyIOParticipantInfo> GetParticipants();

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms", Meta = (AutoCreateRefTerm = "VideoDevice"))
	void EnableVideo(const FDolbyIOVideoDevice& VideoDevice, bool bBlurBackground = false);
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnVideoEnabledDelegate OnVideoEnabled;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnEnableVideoError;

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void DisableVideo();
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnVideoDisabledDelegate OnVideoDisabled;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnDisableVideoError;

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void BindMaterial(UMaterialInstanceDynamic* Material, const FString& VideoTrackID);

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void UnbindMaterial(UMaterialInstanceDynamic* Material, const FString& VideoTrackID);

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	class UTexture2D* GetTexture(const FString& VideoTrackID);

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void GetScreenshareSources();
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnScreenshareSourcesReceivedDelegate OnScreenshareSourcesReceived;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnGetScreenshareSourcesError;

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void StartScreenshare(
	    const FDolbyIOScreenshareSource& Source,
	    EDolbyIOScreenshareEncoderHint EncoderHint = EDolbyIOScreenshareEncoderHint::Detailed,
	    EDolbyIOScreenshareMaxResolution MaxResolution = EDolbyIOScreenshareMaxResolution::ActualCaptured,
	    EDolbyIOScreenshareDownscaleQuality DownscaleQuality = EDolbyIOScreenshareDownscaleQuality::Low);
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnScreenshareStartedDelegate OnScreenshareStarted;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnStartScreenshareError;

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void StopScreenshare();
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnScreenshareStoppedDelegate OnScreenshareStopped;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnStopScreenshareError;

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void ChangeScreenshareParameters(
	    EDolbyIOScreenshareEncoderHint EncoderHint = EDolbyIOScreenshareEncoderHint::Detailed,
	    EDolbyIOScreenshareMaxResolution MaxResolution = EDolbyIOScreenshareMaxResolution::ActualCaptured,
	    EDolbyIOScreenshareDownscaleQuality DownscaleQuality = EDolbyIOScreenshareDownscaleQuality::Low);
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnChangeScreenshareParametersError;

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void GetCurrentScreenshareSource();
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnCurrentScreenshareSourceReceivedDelegate OnCurrentScreenshareSourceReceived;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnGetCurrentScreenshareSourceError;

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetLocalPlayerLocation(const FVector& Location);
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnSetLocalPlayerLocationError;

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetLocalPlayerRotation(const FRotator& Rotation);
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnSetLocalPlayerRotationError;

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetRemotePlayerLocation(const FString& ParticipantID, const FVector& Location);
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnSetRemotePlayerLocationError;

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetLogSettings(EDolbyIOLogLevel SdkLogLevel = EDolbyIOLogLevel::Info,
	                    EDolbyIOLogLevel MediaLogLevel = EDolbyIOLogLevel::Info,
	                    EDolbyIOLogLevel DvcLogLevel = EDolbyIOLogLevel::Info, bool bLogToConsole = false,
	                    bool bLogToFile = true);
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnSetLogSettingsError;

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void GetAudioInputDevices();
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnAudioInputDevicesReceivedDelegate OnAudioInputDevicesReceived;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnGetAudioInputDevicesError;

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void GetAudioOutputDevices();
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnAudioOutputDevicesReceivedDelegate OnAudioOutputDevicesReceived;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnGetAudioOutputDevicesError;

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void GetCurrentAudioInputDevice();
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnCurrentAudioInputDeviceReceivedDelegate OnCurrentAudioInputDeviceReceived;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnGetCurrentAudioInputDeviceError;

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void GetCurrentAudioOutputDevice();
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnCurrentAudioOutputDeviceReceivedDelegate OnCurrentAudioOutputDeviceReceived;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnGetCurrentAudioOutputDeviceError;

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetAudioInputDevice(const FString& NativeID);
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnCurrentAudioInputDeviceChangedDelegate OnCurrentAudioInputDeviceChanged;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnSetAudioInputDeviceError;

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetAudioOutputDevice(const FString& NativeID);
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnCurrentAudioOutputDeviceChangedDelegate OnCurrentAudioOutputDeviceChanged;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnSetAudioOutputDeviceError;

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void GetVideoDevices();
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnVideoDevicesReceivedDelegate OnVideoDevicesReceived;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnGetVideoDevicesError;

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void GetCurrentVideoDevice();
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnCurrentVideoDeviceReceivedDelegate OnCurrentVideoDeviceReceived;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnGetCurrentVideoDeviceError;

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void UpdateUserMetadata(const FString& UserName, const FString& AvatarURL);
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnUpdateUserMetadataError;

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetAudioCaptureMode(EDolbyIONoiseReduction NoiseReduction, EDolbyIOVoiceFont VoiceFont);
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnSetAudioCaptureModeError;

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SendMessage(const FString& Message, const TArray<FString>& ParticipantIDs);
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnSendMessageError;

	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnTokenNeededDelegate OnTokenNeeded;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnParticipantAddedDelegate OnParticipantAdded;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnParticipantUpdatedDelegate OnParticipantUpdated;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnRemoteParticipantConnectedDelegate OnRemoteParticipantConnected;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnRemoteParticipantDisconnectedDelegate OnRemoteParticipantDisconnected;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnLocalParticipantUpdatedDelegate OnLocalParticipantUpdated;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnVideoTrackAddedDelegate OnVideoTrackAdded;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnVideoTrackRemovedDelegate OnVideoTrackRemoved;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnVideoTrackEnabledDelegate OnVideoTrackEnabled;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnVideoTrackDisabledDelegate OnVideoTrackDisabled;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnActiveSpeakersChangedDelegate OnActiveSpeakersChanged;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnAudioLevelsChangedDelegate OnAudioLevelsChanged;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnMessageReceivedDelegate OnMessageReceived;

private:
	void Initialize(FSubsystemCollectionBase&) override;
	void Deinitialize() override;

	bool CanConnect(const FDolbyIOOnErrorDelegate&) const;
	bool IsConnected() const;
	bool IsConnectedAsActive() const;
	bool IsSpatialAudio() const;

	void Initialize(const FString& Token);
	void UpdateStatus(dolbyio::comms::conference_status);
	void EmptyRemoteParticipants();
	void SetSpatialEnvironment();
	void ToggleInputMute();
	void ToggleOutputMute();

	void BroadcastRemoteParticipantConnectedIfNecessary(const FDolbyIOParticipantInfo& ParticipantInfo);
	void BroadcastRemoteParticipantDisconnectedIfNecessary(const FDolbyIOParticipantInfo& ParticipantInfo);

	void BroadcastVideoTrackAdded(const FDolbyIOVideoTrack& VideoTrack);
	void BroadcastVideoTrackEnabled(const FDolbyIOVideoTrack& VideoTrack);
	void ProcessBufferedVideoTracks(const FString& ParticipantID);
	void WarnIfVideoTrackSuspicious(const FString& VideoTrackID);

	void SetLocationUsingFirstPlayer();
	void SetLocalPlayerLocationImpl(const FVector& Location);
	void SetRotationUsingFirstPlayer();
	void SetLocalPlayerRotationImpl(const FRotator& Rotation);

	void Handle(const dolbyio::comms::active_speaker_changed&);
	void Handle(const dolbyio::comms::audio_device_changed&);
	void Handle(const dolbyio::comms::audio_levels&);
	void Handle(const dolbyio::comms::conference_message_received&);
	void Handle(const dolbyio::comms::local_participant_updated&);
	void Handle(const dolbyio::comms::remote_participant_added&);
	void Handle(const dolbyio::comms::remote_participant_updated&);
	void Handle(const dolbyio::comms::remote_video_track_added&);
	void Handle(const dolbyio::comms::remote_video_track_removed&);
	void Handle(const dolbyio::comms::screen_share_error&);
	void Handle(const dolbyio::comms::utils::vfs_event&);

	UDolbyIOSubsystem& GetSubsystem()
	{
		return *this;
	}

	dolbyio::comms::conference_status ConferenceStatus;
	FString LocalParticipantID;
	FString ConferenceID;
	EDolbyIOConnectionMode ConnectionMode;
	EDolbyIOSpatialAudioStyle SpatialAudioStyle;
	TMap<FString, TArray<FDolbyIOVideoTrack>> BufferedAddedVideoTracks;
	TMap<FString, TArray<FDolbyIOVideoTrack>> BufferedEnabledVideoTracks;

	TMap<FString, FDolbyIOParticipantInfo> RemoteParticipants;
	FCriticalSection RemoteParticipantsLock;

	TMap<FString, std::shared_ptr<DolbyIO::FVideoSink>> VideoSinks;
	FCriticalSection VideoSinksLock;

	std::shared_ptr<dolbyio::comms::plugin::video_processor> VideoProcessor;
	std::shared_ptr<DolbyIO::dummy_frame_dropper> LocalCameraFrameHandler;
	std::shared_ptr<DolbyIO::dummy_frame_dropper> LocalScreenshareFrameHandler;
	TSharedPtr<DolbyIO::FDevices> Devices;
	TSharedPtr<dolbyio::comms::sdk> Sdk;
	TSharedPtr<dolbyio::comms::refresh_token> RefreshTokenCb;

	float SpatialEnvironmentScale = 1.0f;

	bool bIsInputMuted = false;
	bool bIsOutputMuted = false;
	bool bIsVideoEnabled = false;

	FTimerHandle LocationTimerHandle;
	FTimerHandle RotationTimerHandle;

	static constexpr auto LocalCameraTrackID = "local-camera";
	static constexpr auto LocalScreenshareTrackID = "local-screenshare";
};

UCLASS(ClassGroup = "Dolby.io Comms",
       Meta = (BlueprintSpawnableComponent, DisplayName = "Dolby.io Observer",
               ToolTip = "Component to use for easy handling of Dolby.io subsystem events."))
class DOLBYIO_API UDolbyIOObserver : public UActorComponent
{
	GENERATED_BODY()

public:
	UDolbyIOObserver()
	{
		bWantsInitializeComponent = true;
	}

	/** Triggered when an initial or refreshed client access token
	 * (https://docs.dolby.io/communications-apis/docs/overview-developer-tools#client-access-token) is needed, which
	 * happens when the Dolby.io Subsystem is initialized or when a refresh token is requested. After receiving this
	 * event, obtain a token for your Dolby.io application and call the Dolby.io Set Token function.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnTokenNeededDelegate OnTokenNeeded;

	/** Triggered when the plugin is successfully initialized after calling the Set Token function. After receiving this
	 * event, the plugin is ready for use. You can now, for example, call the Dolby.io Connect function.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnInitializedDelegate OnInitialized;
	/** Triggered when errors occur after calling the Set Token function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnSetTokenError;

	/** Triggered when the client is successfully connected to the conference after calling the Connect function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnConnectedDelegate OnConnected;
	/** Triggered when errors occur after calling the Connect function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnConnectError;
	/** Triggered when errors occur after calling the Demo Conference function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnDemoConferenceError;

	/** Triggered when the client is disconnected from the conference by any means; in particular, by the Disconnect
	 * function.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnDisconnectedDelegate OnDisconnected;
	/** Triggered when errors occur after calling the Disconnect function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnDisconnectError;

	/** Triggered when errors occur after calling the Set Spatial Environment function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnSetSpatialEnvironmentScaleError;

	/** Triggered when errors occur after calling the Mute Input function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnMuteInputError;

	/** Triggered when errors occur after calling the Unmute Input function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnUnmuteInputError;

	/** Triggered when errors occur after calling the Mute Output function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnMuteOutputError;

	/** Triggered when errors occur after calling the Unmute Output function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnUnmuteOutputError;

	/** Triggered when errors occur after calling the Mute Participant function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnMuteParticipantError;

	/** Triggered when errors occur after calling the Unmute Participant function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnUnmuteParticipantError;

	/** Triggered when a remote participant is added to the conference.
	 *
	 * This event is triggered for all participants who were present in the conference at any time, including
	 * participants who joined and left and are no longer in the conference. The status of the participant provided by
	 * this event should be taken into account when handling this event.
	 *
	 * Users who are only interested in tracking whether a participant is currently connected to the conference are
	 * encouraged to use On Remote Participant Connected and On Remote Participant Disconnected for simplicity.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnParticipantAddedDelegate OnParticipantAdded;

	/** Triggered when a remote participant's status is updated. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnParticipantUpdatedDelegate OnParticipantUpdated;

	/** Triggered when a remote participant is connected to the conference. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnRemoteParticipantConnectedDelegate OnRemoteParticipantConnected;

	/** Triggered when a remote participant is disconnected from the conference. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnRemoteParticipantDisconnectedDelegate OnRemoteParticipantDisconnected;

	/** Triggered when the local participant's status is updated. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnLocalParticipantUpdatedDelegate OnLocalParticipantUpdated;

	/** Triggered when a video track is added. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnVideoTrackAddedDelegate OnVideoTrackAdded;

	/** Triggered when a video track is removed. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnVideoTrackRemovedDelegate OnVideoTrackRemoved;

	/** Triggered when a video track is enabled as a result of the video forwarding strategy. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnVideoTrackEnabledDelegate OnVideoTrackEnabled;

	/** Triggered when a video track is disabled as a result of the video forwarding strategy. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnVideoTrackDisabledDelegate OnVideoTrackDisabled;

	/** Triggered when local video is enabled. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnVideoEnabledDelegate OnVideoEnabled;
	/** Triggered when errors occur after calling the Enable Video function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnEnableVideoError;

	/** Triggered when local video is disabled. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnVideoDisabledDelegate OnVideoDisabled;
	/** Triggered when errors occur after calling the Disable Video function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnDisableVideoError;

	/** Triggered when screen share sources are received as a result of calling Get Screenshare Sources. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnScreenshareSourcesReceivedDelegate OnScreenshareSourcesReceived;
	/** Triggered when errors occur after calling the Get Screenshare Sources function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnGetScreenshareSourcesError;

	/** Triggered when screenshare is started. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnScreenshareStartedDelegate OnScreenshareStarted;
	/** Triggered when errors occur after calling the Start Screenshare function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnStartScreenshareError;

	/** Triggered when screenshare is stopped. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnScreenshareStoppedDelegate OnScreenshareStopped;
	/** Triggered when errors occur after calling the Stop Screenshare function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnStopScreenshareError;

	/** Triggered when errors occur after calling the Change Screenshare Parameters function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnChangeScreenshareParametersError;

	/** Triggered when the current screenshare source is received as a result of calling Get Current Screenshare Source.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnCurrentScreenshareSourceReceivedDelegate OnCurrentScreenshareSourceReceived;
	/** Triggered when errors occur after calling the Get Current Screenshare Source function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnGetCurrentScreenshareSourceError;

	/** Triggered when participants start or stop speaking. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnActiveSpeakersChangedDelegate OnActiveSpeakersChanged;

	/** Triggered roughly every 500ms. The event provides two arrays: an array of IDs of the current speakers and an
	 * array of floating point numbers representing each participant's audio level. The order of Audio Levels
	 * corresponds to the order of Active Speakers. A value of 0.0 represents silence and a value of 1.0 represents the
	 * maximum volume. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnAudioLevelsChangedDelegate OnAudioLevelsChanged;

	/** Triggered when errors occur after calling the Set Local Player Location function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnSetLocalPlayerLocationError;

	/** Triggered when errors occur after calling the Set Local Player Rotation function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnSetLocalPlayerRotationError;

	/** Triggered when errors occur after calling the Set Remote Player Location function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnSetRemotePlayerLocationError;

	/** Triggered when errors occur after calling the Set Log Settings function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnSetLogSettingsError;

	/** Triggered when audio input devices are received as a result of calling Get Audio Input Devices. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnAudioInputDevicesReceivedDelegate OnAudioInputDevicesReceived;
	/** Triggered when errors occur after calling the Get Audio Input Devices function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnGetAudioInputDevicesError;

	/** Triggered when audio output devices are received as a result of calling Get Audio Output Devices. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnAudioOutputDevicesReceivedDelegate OnAudioOutputDevicesReceived;
	/** Triggered when errors occur after calling the Get Audio Output Devices function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnGetAudioOutputDevicesError;

	/** Triggered when the current audio input device is received as a result of calling Get Current Audio Input Device.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnCurrentAudioInputDeviceReceivedDelegate OnCurrentAudioInputDeviceReceived;
	/** Triggered when errors occur after calling the Get Current Audio Input Device function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnGetCurrentAudioInputDeviceError;

	/** Triggered when the current audio output device is received as a result of calling Get Current Audio Output
	 * Device.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnCurrentAudioOutputDeviceReceivedDelegate OnCurrentAudioOutputDeviceReceived;
	/** Triggered when errors occur after calling the Get Current Audio Output Device function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnGetCurrentAudioOutputDeviceError;

	/** Triggered when video devices are received as a result of calling Get Video Devices. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnVideoDevicesReceivedDelegate OnVideoDevicesReceived;
	/** Triggered when errors occur after calling the Get Video Devices function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnGetVideoDevicesError;

	/** Triggered when the current video device is received as a result of calling Get Current Video Device. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnCurrentVideoDeviceReceivedDelegate OnCurrentVideoDeviceReceived;
	/** Triggered when errors occur after calling the Get Current Video Device function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnGetCurrentVideoDeviceError;

	/** Triggered when the current audio input device is changed by the user or as a result of calling Set Audio Input
	 * Device. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnCurrentAudioInputDeviceChangedDelegate OnCurrentAudioInputDeviceChanged;
	/** Triggered when errors occur after calling the Set Audio Input Device function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnSetAudioInputDeviceError;

	/** Triggered when the current audio output device is changed by the user or as a result of calling Set Audio Output
	 * Device. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnCurrentAudioOutputDeviceChangedDelegate OnCurrentAudioOutputDeviceChanged;
	/** Triggered when errors occur after calling the Set Audio Output Device function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnSetAudioOutputDeviceError;

	/** Triggered when errors occur after calling the Update User Metadata function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnUpdateUserMetadataError;

	/** Triggered when errors occur after calling the Set Audio Capture Mode function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnSetAudioCaptureModeError;

	/** Triggered when errors occur after calling the Send Message function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnErrorDelegate OnSendMessageError;

	/** Triggered when a message is received. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnMessageReceivedDelegate OnMessageReceived;

private:
	void InitializeComponent() override;

#define DLB_DEFINE_FORWARDER(Event, ...) \
	{                                    \
		Event.Broadcast(__VA_ARGS__);    \
	}

	UFUNCTION()
	void FwdOnTokenNeeded() DLB_DEFINE_FORWARDER(OnTokenNeeded);

	UFUNCTION() void FwdOnInitialized() DLB_DEFINE_FORWARDER(OnInitialized);
	UFUNCTION() void FwdOnSetTokenError(const FString& ErrorMsg) DLB_DEFINE_FORWARDER(OnSetTokenError, ErrorMsg);

	UFUNCTION()
	void FwdOnConnected(const FString& LocalParticipantID, const FString& ConferenceID)
	    DLB_DEFINE_FORWARDER(OnConnected, LocalParticipantID, ConferenceID);
	UFUNCTION()
	void FwdOnConnectError(const FString& ErrorMsg) DLB_DEFINE_FORWARDER(OnConnectError, ErrorMsg);
	UFUNCTION()
	void FwdOnDemoConferenceError(const FString& ErrorMsg) DLB_DEFINE_FORWARDER(OnDemoConferenceError, ErrorMsg);

	UFUNCTION()
	void FwdOnDisconnected() DLB_DEFINE_FORWARDER(OnDisconnected);
	UFUNCTION()
	void FwdOnDisconnectError(const FString& ErrorMsg) DLB_DEFINE_FORWARDER(OnDisconnectError, ErrorMsg);

	UFUNCTION()
	void FwdOnSetSpatialEnvironmentScaleError(const FString& ErrorMsg)
	    DLB_DEFINE_FORWARDER(OnSetSpatialEnvironmentScaleError, ErrorMsg);

	UFUNCTION()
	void FwdOnMuteInputError(const FString& ErrorMsg) DLB_DEFINE_FORWARDER(OnMuteInputError, ErrorMsg);

	UFUNCTION()
	void FwdOnUnmuteInputError(const FString& ErrorMsg) DLB_DEFINE_FORWARDER(OnUnmuteInputError, ErrorMsg);

	UFUNCTION()
	void FwdOnMuteOutputError(const FString& ErrorMsg) DLB_DEFINE_FORWARDER(OnMuteOutputError, ErrorMsg);

	UFUNCTION()
	void FwdOnUnmuteOutputError(const FString& ErrorMsg) DLB_DEFINE_FORWARDER(OnUnmuteOutputError, ErrorMsg);

	UFUNCTION()
	void FwdOnMuteParticipantError(const FString& ErrorMsg) DLB_DEFINE_FORWARDER(OnMuteParticipantError, ErrorMsg);

	UFUNCTION()
	void FwdOnUnmuteParticipantError(const FString& ErrorMsg) DLB_DEFINE_FORWARDER(OnUnmuteParticipantError, ErrorMsg);

	UFUNCTION()
	void FwdOnParticipantAdded(const EDolbyIOParticipantStatus Status, const FDolbyIOParticipantInfo& ParticipantInfo)
	    DLB_DEFINE_FORWARDER(OnParticipantAdded, Status, ParticipantInfo);

	UFUNCTION()
	void FwdOnParticipantUpdated(const EDolbyIOParticipantStatus Status, const FDolbyIOParticipantInfo& ParticipantInfo)
	    DLB_DEFINE_FORWARDER(OnParticipantUpdated, Status, ParticipantInfo);

	UFUNCTION()
	void FwdOnRemoteParticipantConnected(const FDolbyIOParticipantInfo& ParticipantInfo)
	    DLB_DEFINE_FORWARDER(OnRemoteParticipantConnected, ParticipantInfo);

	UFUNCTION()
	void FwdOnRemoteParticipantDisconnected(const FDolbyIOParticipantInfo& ParticipantInfo)
	    DLB_DEFINE_FORWARDER(OnRemoteParticipantDisconnected, ParticipantInfo);

	UFUNCTION()
	void FwdOnLocalParticipantUpdated(const EDolbyIOParticipantStatus Status,
	                                  const FDolbyIOParticipantInfo& ParticipantInfo)
	    DLB_DEFINE_FORWARDER(OnLocalParticipantUpdated, Status, ParticipantInfo);

	UFUNCTION()
	void FwdOnVideoTrackAdded(const FDolbyIOVideoTrack& VideoTrack) DLB_DEFINE_FORWARDER(OnVideoTrackAdded, VideoTrack);

	UFUNCTION()
	void FwdOnVideoTrackRemoved(const FDolbyIOVideoTrack& VideoTrack)
	    DLB_DEFINE_FORWARDER(OnVideoTrackRemoved, VideoTrack);

	UFUNCTION()
	void FwdOnVideoTrackEnabled(const FDolbyIOVideoTrack& VideoTrack)
	    DLB_DEFINE_FORWARDER(OnVideoTrackEnabled, VideoTrack);

	UFUNCTION()
	void FwdOnVideoTrackDisabled(const FDolbyIOVideoTrack& VideoTrack)
	    DLB_DEFINE_FORWARDER(OnVideoTrackDisabled, VideoTrack);

	UFUNCTION()
	void FwdOnVideoEnabled(const FString& VideoTrackID) DLB_DEFINE_FORWARDER(OnVideoEnabled, VideoTrackID);
	UFUNCTION()
	void FwdOnEnableVideoError(const FString& ErrorMsg) DLB_DEFINE_FORWARDER(OnEnableVideoError, ErrorMsg);

	UFUNCTION()
	void FwdOnVideoDisabled(const FString& VideoTrackID) DLB_DEFINE_FORWARDER(OnVideoDisabled, VideoTrackID);
	UFUNCTION()
	void FwdOnDisableVideoError(const FString& ErrorMsg) DLB_DEFINE_FORWARDER(OnDisableVideoError, ErrorMsg);

	UFUNCTION()
	void FwdOnScreenshareSourcesReceived(const TArray<FDolbyIOScreenshareSource>& Sources)
	    DLB_DEFINE_FORWARDER(OnScreenshareSourcesReceived, Sources);
	UFUNCTION()
	void FwdOnGetScreenshareSourcesError(const FString& ErrorMsg)
	    DLB_DEFINE_FORWARDER(OnGetScreenshareSourcesError, ErrorMsg);

	UFUNCTION()
	void FwdOnScreenshareStarted(const FString& VideoTrackID) DLB_DEFINE_FORWARDER(OnScreenshareStarted, VideoTrackID);
	UFUNCTION()
	void FwdOnStartScreenshareError(const FString& ErrorMsg) DLB_DEFINE_FORWARDER(OnStartScreenshareError, ErrorMsg);

	UFUNCTION()
	void FwdOnScreenshareStopped(const FString& VideoTrackID) DLB_DEFINE_FORWARDER(OnScreenshareStopped, VideoTrackID);
	UFUNCTION()
	void FwdOnStopScreenshareError(const FString& ErrorMsg) DLB_DEFINE_FORWARDER(OnStopScreenshareError, ErrorMsg);

	UFUNCTION()
	void FwdOnChangeScreenshareParametersError(const FString& ErrorMsg)
	    DLB_DEFINE_FORWARDER(OnChangeScreenshareParametersError, ErrorMsg);

	UFUNCTION()
	void FwdOnCurrentScreenshareSourceReceived(bool IsNone, const FDolbyIOScreenshareSource& OptionalSource)
	    DLB_DEFINE_FORWARDER(OnCurrentScreenshareSourceReceived, IsNone, OptionalSource);
	UFUNCTION()
	void FwdOnGetCurrentScreenshareSourceError(const FString& ErrorMsg)
	    DLB_DEFINE_FORWARDER(OnGetCurrentScreenshareSourceError, ErrorMsg);

	UFUNCTION()
	void FwdOnActiveSpeakersChanged(const TArray<FString>& ActiveSpeakers)
	    DLB_DEFINE_FORWARDER(OnActiveSpeakersChanged, ActiveSpeakers);

	UFUNCTION()
	void FwdOnAudioLevelsChanged(const TArray<FString>& ActiveSpeakers, const TArray<float>& AudioLevels)
	    DLB_DEFINE_FORWARDER(OnAudioLevelsChanged, ActiveSpeakers, AudioLevels);

	UFUNCTION()
	void FwdOnSetLocalPlayerLocationError(const FString& ErrorMsg)
	    DLB_DEFINE_FORWARDER(OnSetLocalPlayerLocationError, ErrorMsg);

	UFUNCTION()
	void FwdOnSetLocalPlayerRotationError(const FString& ErrorMsg)
	    DLB_DEFINE_FORWARDER(OnSetLocalPlayerRotationError, ErrorMsg);

	UFUNCTION()
	void FwdOnSetRemotePlayerLocationError(const FString& ErrorMsg)
	    DLB_DEFINE_FORWARDER(OnSetRemotePlayerLocationError, ErrorMsg);

	UFUNCTION()
	void FwdOnSetLogSettingsError(const FString& ErrorMsg) DLB_DEFINE_FORWARDER(OnSetLogSettingsError, ErrorMsg);

	UFUNCTION()
	void FwdOnAudioInputDevicesReceived(const TArray<FDolbyIOAudioDevice>& Devices)
	    DLB_DEFINE_FORWARDER(OnAudioInputDevicesReceived, Devices);
	UFUNCTION()
	void FwdOnGetAudioInputDevicesError(const FString& ErrorMsg)
	    DLB_DEFINE_FORWARDER(OnGetAudioInputDevicesError, ErrorMsg);

	UFUNCTION()
	void FwdOnAudioOutputDevicesReceived(const TArray<FDolbyIOAudioDevice>& Devices)
	    DLB_DEFINE_FORWARDER(OnAudioOutputDevicesReceived, Devices);
	UFUNCTION()
	void FwdOnGetAudioOutputDevicesError(const FString& ErrorMsg)
	    DLB_DEFINE_FORWARDER(OnGetAudioOutputDevicesError, ErrorMsg);

	UFUNCTION()
	void FwdOnCurrentAudioInputDeviceReceived(bool IsNone, const FDolbyIOAudioDevice& OptionalDevice)
	    DLB_DEFINE_FORWARDER(OnCurrentAudioInputDeviceReceived, IsNone, OptionalDevice);
	UFUNCTION()
	void FwdOnGetCurrentAudioInputDeviceError(const FString& ErrorMsg)
	    DLB_DEFINE_FORWARDER(OnGetCurrentAudioInputDeviceError, ErrorMsg);

	UFUNCTION()
	void FwdOnCurrentAudioOutputDeviceReceived(bool IsNone, const FDolbyIOAudioDevice& OptionalDevice)
	    DLB_DEFINE_FORWARDER(OnCurrentAudioOutputDeviceReceived, IsNone, OptionalDevice);
	UFUNCTION()
	void FwdOnGetCurrentAudioOutputDeviceError(const FString& ErrorMsg)
	    DLB_DEFINE_FORWARDER(OnGetCurrentAudioOutputDeviceError, ErrorMsg);

	UFUNCTION()
	void FwdOnVideoDevicesReceived(const TArray<FDolbyIOVideoDevice>& Devices)
	    DLB_DEFINE_FORWARDER(OnVideoDevicesReceived, Devices);
	UFUNCTION()
	void FwdOnGetVideoDevicesError(const FString& ErrorMsg) DLB_DEFINE_FORWARDER(OnGetVideoDevicesError, ErrorMsg);

	UFUNCTION()
	void FwdOnCurrentVideoDeviceReceived(bool IsNone, const FDolbyIOVideoDevice& OptionalDevice)
	    DLB_DEFINE_FORWARDER(OnCurrentVideoDeviceReceived, IsNone, OptionalDevice);
	UFUNCTION()
	void FwdOnGetCurrentVideoDeviceError(const FString& ErrorMsg)
	    DLB_DEFINE_FORWARDER(OnGetCurrentVideoDeviceError, ErrorMsg);

	UFUNCTION()
	void FwdOnCurrentAudioInputDeviceChanged(bool IsNone, const FDolbyIOAudioDevice& OptionalDevice)
	    DLB_DEFINE_FORWARDER(OnCurrentAudioInputDeviceChanged, IsNone, OptionalDevice);
	UFUNCTION()
	void FwdOnSetAudioInputDeviceError(const FString& ErrorMsg)
	    DLB_DEFINE_FORWARDER(OnSetAudioInputDeviceError, ErrorMsg);

	UFUNCTION()
	void FwdOnCurrentAudioOutputDeviceChanged(bool IsNone, const FDolbyIOAudioDevice& OptionalDevice)
	    DLB_DEFINE_FORWARDER(OnCurrentAudioOutputDeviceChanged, IsNone, OptionalDevice);
	UFUNCTION()
	void FwdOnSetAudioOutputDeviceError(const FString& ErrorMsg)
	    DLB_DEFINE_FORWARDER(OnSetAudioOutputDeviceError, ErrorMsg);

	UFUNCTION()
	void FwdOnUpdateUserMetadataError(const FString& ErrorMsg)
	    DLB_DEFINE_FORWARDER(OnUpdateUserMetadataError, ErrorMsg);

	UFUNCTION()
	void FwdOnSetAudioCaptureModeError(const FString& ErrorMsg)
	    DLB_DEFINE_FORWARDER(OnSetAudioCaptureModeError, ErrorMsg);

	UFUNCTION()
	void FwdOnSendMessageError(const FString& ErrorMsg) DLB_DEFINE_FORWARDER(OnSendMessageError, ErrorMsg);

	UFUNCTION()
	void FwdOnMessageReceived(const FString& Message, const FDolbyIOParticipantInfo& ParticipantInfo)
	    DLB_DEFINE_FORWARDER(OnMessageReceived, Message, ParticipantInfo);

#undef DLB_DEFINE_FORWARDER
};
