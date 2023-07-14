// Copyright 2023 Dolby Laboratories

#pragma once

#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "DolbyIOTypes.h"

#include <memory>

#include "DolbyIO.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDolbyIOOnTokenNeededDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDolbyIOOnInitializedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDolbyIOOnConnectedDelegate, const FString&, LocalParticipantID,
                                             const FString&, ConferenceID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDolbyIOOnDisconnectedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDolbyIOOnParticipantAddedDelegate, const EDolbyIOParticipantStatus,
                                             Status, const FDolbyIOParticipantInfo&, ParticipantInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDolbyIOOnParticipantUpdatedDelegate, const EDolbyIOParticipantStatus,
                                             Status, const FDolbyIOParticipantInfo&, ParticipantInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDolbyIOOnVideoTrackAddedDelegate, const FDolbyIOVideoTrack&, VideoTrack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDolbyIOOnVideoTrackRemovedDelegate, const FDolbyIOVideoTrack&, VideoTrack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDolbyIOOnVideoTrackEnabledDelegate, const FDolbyIOVideoTrack&, VideoTrack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDolbyIOOnVideoTrackDisabledDelegate, const FDolbyIOVideoTrack&,
                                            VideoTrack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDolbyIOOnVideoEnabledDelegate, const FString&, VideoTrackID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDolbyIOOnVideoDisabledDelegate, const FString&, VideoTrackID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDolbyIOOnScreenshareStartedDelegate, const FString&, VideoTrackID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDolbyIOOnScreenshareStoppedDelegate, const FString&, VideoTrackID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDolbyIOOnActiveSpeakersChangedDelegate, const TArray<FString>&,
                                            ActiveSpeakers);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDolbyIOOnAudioLevelsChangedDelegate, const TArray<FString>&,
                                             ActiveSpeakers, const TArray<float>&, AudioLevels);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDolbyIOOnScreenshareSourcesReceivedDelegate,
                                            const TArray<FDolbyIOScreenshareSource>&, Sources);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDolbyIOOnAudioInputDevicesReceivedDelegate,
                                            const TArray<FDolbyIOAudioDevice>&, Devices);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDolbyIOOnAudioOutputDevicesReceivedDelegate,
                                            const TArray<FDolbyIOAudioDevice>&, Devices);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDolbyIOOnCurrentAudioInputDeviceReceivedDelegate, bool, IsNone,
                                             const FDolbyIOAudioDevice&, OptionalDevice);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDolbyIOOnCurrentAudioOutputDeviceReceivedDelegate, bool, IsNone,
                                             const FDolbyIOAudioDevice&, OptionalDevice);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDolbyIOOnVideoDevicesReceivedDelegate, const TArray<FDolbyIOVideoDevice>&,
                                            Devices);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDolbyIOOnCurrentAudioInputDeviceChangedDelegate, bool, IsNone,
                                             const FDolbyIOAudioDevice&, OptionalDevice);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDolbyIOOnCurrentAudioOutputDeviceChangedDelegate, bool, IsNone,
                                             const FDolbyIOAudioDevice&, OptionalDevice);

namespace dolbyio::comms
{
	enum class conference_status;
	class refresh_token;
	class sdk;

	namespace plugin
	{
		class video_processor;
	}
}

namespace DolbyIO
{
	class FDevices;
	class FErrorHandler;
	class FVideoFrameHandler;
	class FVideoSink;
}

UCLASS(DisplayName = "Dolby.io Subsystem")
class DOLBYIO_API UDolbyIOSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	friend class DolbyIO::FErrorHandler;

public:
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetToken(const FString& Token);

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void Connect(
	    const FString& ConferenceName = "unreal", const FString& UserName = "", const FString& ExternalID = "",
	    const FString& AvatarURL = "", EDolbyIOConnectionMode ConnectionMode = EDolbyIOConnectionMode::Active,
	    EDolbyIOSpatialAudioStyle SpatialAudioStyle = EDolbyIOSpatialAudioStyle::Shared, int MaxVideoStreams = 25,
	    EDolbyIOVideoForwardingStrategy VideoForwardingStrategy = EDolbyIOVideoForwardingStrategy::LastSpeaker);

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void DemoConference();

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void Disconnect();

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetSpatialEnvironmentScale(float SpatialEnvironmentScale = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void MuteInput();

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void UnmuteInput();

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void MuteOutput();

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void UnmuteOutput();

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void MuteParticipant(const FString& ParticipantID);

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void UnmuteParticipant(const FString& ParticipantID);

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	TArray<FDolbyIOParticipantInfo> GetParticipants();

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms", Meta = (AutoCreateRefTerm = "VideoDevice"))
	void EnableVideo(const FDolbyIOVideoDevice& VideoDevice, bool bBlurBackground = false);

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void DisableVideo();

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void BindMaterial(UMaterialInstanceDynamic* Material, const FString& VideoTrackID);

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void UnbindMaterial(UMaterialInstanceDynamic* Material, const FString& VideoTrackID);

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	class UTexture2D* GetTexture(const FString& VideoTrackID);

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void GetScreenshareSources();

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void StartScreenshare(
	    const FDolbyIOScreenshareSource& Source,
	    EDolbyIOScreenshareEncoderHint EncoderHint = EDolbyIOScreenshareEncoderHint::Detailed,
	    EDolbyIOScreenshareMaxResolution MaxResolution = EDolbyIOScreenshareMaxResolution::ActualCaptured,
	    EDolbyIOScreenshareDownscaleQuality DownscaleQuality = EDolbyIOScreenshareDownscaleQuality::Low);

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void StopScreenshare();

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void ChangeScreenshareParameters(
	    EDolbyIOScreenshareEncoderHint EncoderHint = EDolbyIOScreenshareEncoderHint::Detailed,
	    EDolbyIOScreenshareMaxResolution MaxResolution = EDolbyIOScreenshareMaxResolution::ActualCaptured,
	    EDolbyIOScreenshareDownscaleQuality DownscaleQuality = EDolbyIOScreenshareDownscaleQuality::Low);

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetLocalPlayerLocation(const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetLocalPlayerRotation(const FRotator& Rotation);

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetRemotePlayerLocation(const FString& ParticipantID, const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetLogSettings(EDolbyIOLogLevel SdkLogLevel = EDolbyIOLogLevel::Info,
	                    EDolbyIOLogLevel MediaLogLevel = EDolbyIOLogLevel::Info,
	                    EDolbyIOLogLevel DvcLogLevel = EDolbyIOLogLevel::Info);

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void GetAudioInputDevices();

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void GetAudioOutputDevices();

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void GetCurrentAudioInputDevice();

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void GetCurrentAudioOutputDevice();

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetAudioInputDevice(const FString& NativeID);

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetAudioOutputDevice(const FString& NativeID);

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void GetVideoDevices();

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void UpdateUserMetadata(const FString& UserName, const FString& AvatarURL);

	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetAudioCaptureMode(EDolbyIONoiseReduction NoiseReduction, EDolbyIOVoiceFont VoiceFont);

	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnTokenNeededDelegate OnTokenNeeded;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnInitializedDelegate OnInitialized;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnConnectedDelegate OnConnected;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnDisconnectedDelegate OnDisconnected;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnParticipantAddedDelegate OnParticipantAdded;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnParticipantUpdatedDelegate OnParticipantUpdated;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnVideoTrackAddedDelegate OnVideoTrackAdded;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnVideoTrackRemovedDelegate OnVideoTrackRemoved;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnVideoTrackEnabledDelegate OnVideoTrackEnabled;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnVideoTrackDisabledDelegate OnVideoTrackDisabled;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnVideoEnabledDelegate OnVideoEnabled;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnVideoDisabledDelegate OnVideoDisabled;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnScreenshareStartedDelegate OnScreenshareStarted;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnScreenshareStoppedDelegate OnScreenshareStopped;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnActiveSpeakersChangedDelegate OnActiveSpeakersChanged;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnAudioLevelsChangedDelegate OnAudioLevelsChanged;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnScreenshareSourcesReceivedDelegate OnScreenshareSourcesReceived;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnAudioInputDevicesReceivedDelegate OnAudioInputDevicesReceived;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnAudioOutputDevicesReceivedDelegate OnAudioOutputDevicesReceived;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnCurrentAudioInputDeviceReceivedDelegate OnCurrentAudioInputDeviceReceived;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnCurrentAudioOutputDeviceReceivedDelegate OnCurrentAudioOutputDeviceReceived;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnVideoDevicesReceivedDelegate OnVideoDevicesReceived;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnCurrentAudioInputDeviceChangedDelegate OnCurrentAudioInputDeviceChanged;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnCurrentAudioOutputDeviceChangedDelegate OnCurrentAudioOutputDeviceChanged;

private:
	void Initialize(FSubsystemCollectionBase&) override;

	bool CanConnect() const;
	bool IsConnected() const;
	bool IsConnectedAsActive() const;
	bool IsSpatialAudio() const;

	void Initialize(const FString& Token);
	void UpdateStatus(dolbyio::comms::conference_status);
	void EmptyRemoteParticipants();
	void SetSpatialEnvironment();
	void ToggleInputMute();
	void ToggleOutputMute();

	void SetLocationUsingFirstPlayer();
	void SetLocalPlayerLocationImpl(const FVector& Location);
	void SetRotationUsingFirstPlayer();
	void SetLocalPlayerRotationImpl(const FRotator& Rotation);

	dolbyio::comms::conference_status ConferenceStatus;
	FString LocalParticipantID;
	FString ConferenceID;
	EDolbyIOConnectionMode ConnectionMode;
	EDolbyIOSpatialAudioStyle SpatialAudioStyle;
	TMap<FString, TArray<FDolbyIOVideoTrack>> BufferedVideoTracks;
	TMap<FString, FDolbyIOParticipantInfo> RemoteParticipants;
	FCriticalSection RemoteParticipantsLock;

	TMap<FString, std::shared_ptr<DolbyIO::FVideoSink>> VideoSinks;
	std::shared_ptr<dolbyio::comms::plugin::video_processor> VideoProcessor;
	std::shared_ptr<DolbyIO::FVideoFrameHandler> LocalCameraFrameHandler;
	std::shared_ptr<DolbyIO::FVideoFrameHandler> LocalScreenshareFrameHandler;
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

	/** Triggered when the client is successfully connected to the conference after calling the Connect function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnConnectedDelegate OnConnected;

	/** Triggered when the client is disconnected from the conference by any means; in particular, by the Disconnect
	 * function.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnDisconnectedDelegate OnDisconnected;

	/** Triggered when a remote participant is added to the conference. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnParticipantAddedDelegate OnParticipantAdded;

	/** Triggered when a remote participant's status is updated. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnParticipantUpdatedDelegate OnParticipantUpdated;

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

	/** Triggered when local video is disabled. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnVideoDisabledDelegate OnVideoDisabled;

	/** Triggered when screenshare is started. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnScreenshareStartedDelegate OnScreenshareStarted;

	/** Triggered when screenshare is stopped. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnScreenshareStoppedDelegate OnScreenshareStopped;

	/** Triggered when participants start or stop speaking. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnActiveSpeakersChangedDelegate OnActiveSpeakersChanged;

	/** Triggered roughly every 500ms. The event provides two arrays: an array of IDs of the current speakers and an
	 * array of floating point numbers representing each participant's audio level. The order of Audio Levels
	 * corresponds to the order of Active Speakers. A value of 0.0 represents silence and a value of 1.0 represents the
	 * maximum volume. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnAudioLevelsChangedDelegate OnAudioLevelsChanged;

	/** Triggered when screen share sources are received as a result of calling Get Screenshare Sources. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnScreenshareSourcesReceivedDelegate OnScreenshareSourcesReceived;

	/** Triggered when audio input devices are received as a result of calling Get Audio Input Devices. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnAudioInputDevicesReceivedDelegate OnAudioInputDevicesReceived;

	/** Triggered when audio output devices are received as a result of calling Get Audio Output Devices. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnAudioOutputDevicesReceivedDelegate OnAudioOutputDevicesReceived;

	/** Triggered when the current audio input device is received as a result of calling Get Current Audio Input Device.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnCurrentAudioInputDeviceReceivedDelegate OnCurrentAudioInputDeviceReceived;

	/** Triggered when the current audio output device is received as a result of calling Get Current Audio Output
	 * Device.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnCurrentAudioOutputDeviceReceivedDelegate OnCurrentAudioOutputDeviceReceived;

	/** Triggered when video devices are received as a result of calling Get Video Devices. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnVideoDevicesReceivedDelegate OnVideoDevicesReceived;

	/** Triggered when the current audio input device is changed by the user or as a result of calling
	 * Set Current Audio Input Device. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnCurrentAudioInputDeviceChangedDelegate OnCurrentAudioInputDeviceChanged;

	/** Triggered when the current audio output device is changed by the user or as a result of calling
	 * Set Current Audio Output Device. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FDolbyIOOnCurrentAudioOutputDeviceChangedDelegate OnCurrentAudioOutputDeviceChanged;

private:
	void InitializeComponent() override;

#define DLB_DEFINE_FORWARDER(Event, ...) \
	{                                    \
		Event.Broadcast(__VA_ARGS__);    \
	}

	UFUNCTION()
	void FwdOnTokenNeeded() DLB_DEFINE_FORWARDER(OnTokenNeeded);

	UFUNCTION() void FwdOnInitialized() DLB_DEFINE_FORWARDER(OnInitialized);

	UFUNCTION()
	void FwdOnConnected(const FString& LocalParticipantID, const FString& ConferenceID)
	    DLB_DEFINE_FORWARDER(OnConnected, LocalParticipantID, ConferenceID);

	UFUNCTION()
	void FwdOnDisconnected() DLB_DEFINE_FORWARDER(OnDisconnected);

	UFUNCTION()
	void FwdOnParticipantAdded(const EDolbyIOParticipantStatus Status, const FDolbyIOParticipantInfo& ParticipantInfo)
	    DLB_DEFINE_FORWARDER(OnParticipantAdded, Status, ParticipantInfo);

	UFUNCTION()
	void FwdOnParticipantUpdated(const EDolbyIOParticipantStatus Status, const FDolbyIOParticipantInfo& ParticipantInfo)
	    DLB_DEFINE_FORWARDER(OnParticipantUpdated, Status, ParticipantInfo);

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
	void FwdOnVideoDisabled(const FString& VideoTrackID) DLB_DEFINE_FORWARDER(OnVideoDisabled, VideoTrackID);

	UFUNCTION()
	void FwdOnScreenshareStarted(const FString& VideoTrackID) DLB_DEFINE_FORWARDER(OnScreenshareStarted, VideoTrackID);

	UFUNCTION()
	void FwdOnScreenshareStopped(const FString& VideoTrackID) DLB_DEFINE_FORWARDER(OnScreenshareStopped, VideoTrackID);

	UFUNCTION()
	void FwdOnActiveSpeakersChanged(const TArray<FString>& ActiveSpeakers)
	    DLB_DEFINE_FORWARDER(OnActiveSpeakersChanged, ActiveSpeakers);

	UFUNCTION()
	void FwdOnAudioLevelsChanged(const TArray<FString>& ActiveSpeakers, const TArray<float>& AudioLevels)
	    DLB_DEFINE_FORWARDER(OnAudioLevelsChanged, ActiveSpeakers, AudioLevels);

	UFUNCTION()
	void FwdOnScreenshareSourcesReceived(const TArray<FDolbyIOScreenshareSource>& Sources)
	    DLB_DEFINE_FORWARDER(OnScreenshareSourcesReceived, Sources);

	UFUNCTION()
	void FwdOnAudioInputDevicesReceived(const TArray<FDolbyIOAudioDevice>& Devices)
	    DLB_DEFINE_FORWARDER(OnAudioInputDevicesReceived, Devices);

	UFUNCTION()
	void FwdOnAudioOutputDevicesReceived(const TArray<FDolbyIOAudioDevice>& Devices)
	    DLB_DEFINE_FORWARDER(OnAudioOutputDevicesReceived, Devices);

	UFUNCTION()
	void FwdOnCurrentAudioInputDeviceReceived(bool IsNone, const FDolbyIOAudioDevice& OptionalDevice)
	    DLB_DEFINE_FORWARDER(OnCurrentAudioInputDeviceReceived, IsNone, OptionalDevice);

	UFUNCTION()
	void FwdOnCurrentAudioOutputDeviceReceived(bool IsNone, const FDolbyIOAudioDevice& OptionalDevice)
	    DLB_DEFINE_FORWARDER(OnCurrentAudioOutputDeviceReceived, IsNone, OptionalDevice);

	UFUNCTION()
	void FwdOnVideoDevicesReceived(const TArray<FDolbyIOVideoDevice>& Devices)
	    DLB_DEFINE_FORWARDER(OnVideoDevicesReceived, Devices);

	UFUNCTION()
	void FwdOnCurrentAudioInputDeviceChanged(bool IsNone, const FDolbyIOAudioDevice& OptionalDevice)
	    DLB_DEFINE_FORWARDER(OnCurrentAudioInputDeviceChanged, IsNone, OptionalDevice);

	UFUNCTION()
	void FwdOnCurrentAudioOutputDeviceChanged(bool IsNone, const FDolbyIOAudioDevice& OptionalDevice)
	    DLB_DEFINE_FORWARDER(OnCurrentAudioOutputDeviceChanged, IsNone, OptionalDevice);

#undef DLB_DEFINE_FORWARDER
};
