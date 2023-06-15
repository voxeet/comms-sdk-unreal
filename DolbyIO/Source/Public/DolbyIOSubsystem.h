// Copyright 2023 Dolby Laboratories

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"

#include "DolbyIOConnectionMode.h"
#include "DolbyIODevices.h"
#include "DolbyIOLogLevel.h"
#include "DolbyIOParticipantInfo.h"
#include "DolbyIOScreenshareSource.h"
#include "DolbyIOSpatialAudioStyle.h"
#include "DolbyIOVideoForwardingStrategy.h"
#include "DolbyIOVideoTrack.h"

#include <memory>

#include "Engine/EngineTypes.h"

#include "DolbyIOSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSubsystemOnTokenNeededDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSubsystemOnInitializedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSubsystemOnConnectedDelegate, const FString&, LocalParticipantID,
                                             const FString&, ConferenceID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSubsystemOnDisconnectedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSubsystemOnParticipantAddedDelegate, const EDolbyIOParticipantStatus,
                                             Status, const FDolbyIOParticipantInfo&, ParticipantInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSubsystemOnParticipantUpdatedDelegate, const EDolbyIOParticipantStatus,
                                             Status, const FDolbyIOParticipantInfo&, ParticipantInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubsystemOnVideoTrackAddedDelegate, const FDolbyIOVideoTrack&, VideoTrack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubsystemOnVideoTrackRemovedDelegate, const FDolbyIOVideoTrack&,
                                            VideoTrack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubsystemOnVideoTrackEnabledDelegate, const FDolbyIOVideoTrack&,
                                            VideoTrack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubsystemOnVideoTrackDisabledDelegate, const FDolbyIOVideoTrack&,
                                            VideoTrack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubsystemOnVideoEnabledDelegate, const FString&, VideoTrackID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubsystemOnVideoDisabledDelegate, const FString&, VideoTrackID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubsystemOnScreenshareStartedDelegate, const FString&, VideoTrackID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubsystemOnScreenshareStoppedDelegate, const FString&, VideoTrackID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubsystemOnActiveSpeakersChangedDelegate, const TArray<FString>&,
                                            ActiveSpeakers);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSubsystemOnAudioLevelsChangedDelegate, const TArray<FString>&,
                                             ActiveSpeakers, const TArray<float>&, AudioLevels);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubsystemOnScreenshareSourcesReceivedDelegate,
                                            const TArray<FDolbyIOScreenshareSource>&, Sources);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubsystemOnAudioInputDevicesReceivedDelegate,
                                            const TArray<FDolbyIOAudioDevice>&, Devices);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubsystemOnAudioOutputDevicesReceivedDelegate,
                                            const TArray<FDolbyIOAudioDevice>&, Devices);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSubsystemOnCurrentAudioInputDeviceReceivedDelegate, bool, IsNone,
                                             const FDolbyIOAudioDevice&, OptionalDevice);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSubsystemOnCurrentAudioOutputDeviceReceivedDelegate, bool, IsNone,
                                             const FDolbyIOAudioDevice&, OptionalDevice);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubsystemOnVideoDevicesReceivedDelegate,
                                            const TArray<FDolbyIOVideoDevice>&, Devices);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSubsystemOnCurrentAudioInputDeviceChangedDelegate, bool, IsNone,
                                             const FDolbyIOAudioDevice&, OptionalDevice);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSubsystemOnCurrentAudioOutputDeviceChangedDelegate, bool, IsNone,
                                             const FDolbyIOAudioDevice&, OptionalDevice);

namespace dolbyio::comms
{
	enum class conference_status;
	class refresh_token;
	class sdk;
}

namespace DolbyIO
{
	class FVideoFrameHandler;
	class FVideoSink;
}

UCLASS(DisplayName = "Dolby.io Subsystem")
class DOLBYIO_API UDolbyIOSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Initializes or refreshes the client access token. Initializes the plugin unless already initialized.
	 *
	 * Successful initialization triggers the On Initialized event.
	 *
	 * For quick testing, you can manually obtain a token from the Dolby.io dashboard (https://dashboard.dolby.io) and
	 * paste it directly into the node or use the Get Dolby.io Token function.
	 *
	 * @param Token - The client access token.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetToken(const FString& Token);

	/** Connects to a conference.
	 *
	 * Triggers On Connected if successful.
	 *
	 * @param ConferenceName - The conference name. Must not be empty.
	 * @param UserName - The name of the participant.
	 * @param ExternalID - The external unique identifier that the customer's application can add to the participant
	 * while opening a session. If a participant uses the same external ID in conferences, the participant's ID also
	 * remains the same across all sessions.
	 * @param AvatarURL - The URL of the participant's avatar.
	 * @param ConnectionMode - Defines whether to connect as an active user or a listener.
	 * @param SpatialAudioStyle - The spatial audio style of the conference.
	 * @param MaxVideoStreams - Sets the maximum number of video streams that may be transmitted to the user. Valid
	 * parameter values are between 0 and 25.
	 * @param VideoForwardingStrategy - Defines how the plugin should select conference participants whose videos will
	 * be transmitted to the local participant.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void Connect(
	    const FString& ConferenceName = "unreal", const FString& UserName = "", const FString& ExternalID = "",
	    const FString& AvatarURL = "", EDolbyIOConnectionMode ConnectionMode = EDolbyIOConnectionMode::Active,
	    EDolbyIOSpatialAudioStyle SpatialAudioStyle = EDolbyIOSpatialAudioStyle::Shared, int MaxVideoStreams = 25,
	    EDolbyIOVideoForwardingStrategy VideoForwardingStrategy = EDolbyIOVideoForwardingStrategy::LastSpeaker);

	/** Connects to a demo conference.
	 *
	 * The demo automatically brings in 3 invisible bots into the conference as a quick way to validate the connection
	 * to the service with audio functionality. The bots are placed at point {0, 0, 0}.
	 *
	 * Triggers On Connected if successful.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void DemoConference();

	/** Disconnects from the current conference.
	 *
	 * Triggers On Disconnected if successful.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void Disconnect();

	/** Sets the spatial environment scale.
	 *
	 * The larger the scale, the longer the distance at which the spatial audio
	 * attenuates. To get the best experience, the scale should be set separately for each level. The default value of
	 * "1.0" means that audio will fall completely silent at a distance of 10000 units (10000 cm/100 m).
	 *
	 * @param SpatialEnvironmentScale - The scale as a floating point number.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetSpatialEnvironmentScale(float SpatialEnvironmentScale = 1.0f);

	/** Mutes audio input. */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void MuteInput();

	/** Unmutes audio input. */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void UnmuteInput();

	/** Mutes audio output. */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void MuteOutput();

	/** Unmutes audio output. */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void UnmuteOutput();

	/** Mutes a given participant for the local user.
	 *
	 * @param ParticipantID - The ID of the remote participant to mute.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void MuteParticipant(const FString& ParticipantID);

	/** Unmutes a given participant for the local user.
	 *
	 * @param ParticipantID - The ID of the remote participant to unmute.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void UnmuteParticipant(const FString& ParticipantID);

	/** Gets a list of all remote participants.
	 *
	 * @return An array of current Dolby.io Participant Info's.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	TArray<FDolbyIOParticipantInfo> GetParticipants();

	/** Enables video streaming from the given video device or the default device if no device is given.
	 *
	 * Triggers On Video Enabled if successful.
	 *
	 * @param VideoDevice - The video device to use.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms", Meta = (AutoCreateRefTerm = "VideoDevice"))
	void EnableVideo(const FDolbyIOVideoDevice& VideoDevice);

	/** Disables video streaming.
	 *
	 * Triggers On Video Disabled if successful.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void DisableVideo();

	/** Binds a dynamic material instance to hold the frames of the given video track. The plugin will update the
	 * material's texture parameter named "DolbyIO Frame" with the necessary data, therefore the material should have
	 * such a parameter to be usable. Automatically unbinds the material from all other tracks, but it is possible to
	 * bind multiple materials to the same track. Has no effect if the track does not exist at the moment the function
	 * is called, therefore it should usually be called as a response to the "On Video Track Added" event.
	 *
	 * @param Material - The dynamic material instance to bind.
	 * @param VideoTrackID - The ID of the video track.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void BindMaterial(UMaterialInstanceDynamic* Material, const FString& VideoTrackID);

	/** Unbinds a dynamic material instance to no longer hold the video frames of the given video track. The plugin will
	 * no longer update the material's texture parameter named "DolbyIO Frame" with the necessary data.
	 *
	 * @param Material - The dynamic material instance to unbind.
	 * @param VideoTrackID - The ID of the video track.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void UnbindMaterial(UMaterialInstanceDynamic* Material, const FString& VideoTrackID);

	/** Gets the texture to which video from a given track is being rendered.
	 *
	 * @param VideoTrackID - The ID of the video track.
	 * @return The texture holding the video track's frame or NULL if no such texture exists.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	class UTexture2D* GetTexture(const FString& VideoTrackID);

	/** Gets a list of all possible screen sharing sources. These can be entire screens or specific application windows.
	 *
	 * Triggers On Screenshare Sources Received if successful.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void GetScreenshareSources();

	/** Starts screen sharing using a given source.
	 *
	 * Users should make use of the parameters to optimize for the content they are sharing. For example, for sharing
	 * dynamic content like a YouTube video, the ideal settings are MaxResolution=DownscaleTo1080p, EncoderHint=Fluid,
	 * DownscaleQuality=High.
	 *
	 * Triggers On Screenshare Started if successful.
	 *
	 * @param Source - The source to use.
	 * @param EncoderHint - Provides a hint to the plugin as to what type of content is being captured by the screen
	 * share.
	 * @param MaxResolution - The maximum resolution for the capture screen content to be shared as.
	 * @param DownscaleQuality - The quality for the downscaling algorithm to be used.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void StartScreenshare(
	    const FDolbyIOScreenshareSource& Source,
	    EDolbyIOScreenshareEncoderHint EncoderHint = EDolbyIOScreenshareEncoderHint::Detailed,
	    EDolbyIOScreenshareMaxResolution MaxResolution = EDolbyIOScreenshareMaxResolution::ActualCaptured,
	    EDolbyIOScreenshareDownscaleQuality DownscaleQuality = EDolbyIOScreenshareDownscaleQuality::Low);

	/** Stops screen sharing.
	 *
	 * Triggers On Screenshare Stopped if successful.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void StopScreenshare();

	/** Changes the screen sharing parameters if already sharing screen.
	 *
	 * @param EncoderHint - Provides a hint to the plugin as to what type of content is being captured by the screen
	 * share.
	 * @param MaxResolution - The maximum resolution for the capture screen content to be shared as.
	 * @param DownscaleQuality - The quality for the downscaling algorithm to be used.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void ChangeScreenshareParameters(
	    EDolbyIOScreenshareEncoderHint EncoderHint = EDolbyIOScreenshareEncoderHint::Detailed,
	    EDolbyIOScreenshareMaxResolution MaxResolution = EDolbyIOScreenshareMaxResolution::ActualCaptured,
	    EDolbyIOScreenshareDownscaleQuality DownscaleQuality = EDolbyIOScreenshareDownscaleQuality::Low);

	/** Updates the location of the listener for spatial audio purposes.
	 *
	 * Calling this function even once disables the default behavior, which is to automatically use the location of the
	 * first player controller.
	 *
	 * @param Location - The location of the listener.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetLocalPlayerLocation(const FVector& Location);

	/** Updates the rotation of the listener for spatial audio purposes.
	 *
	 * Calling this function even once disables the default behavior, which is to automatically use the rotation of the
	 * first player controller.
	 *
	 * @param Rotation - The rotation of the listener.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetLocalPlayerRotation(const FRotator& Rotation);

	/** Updates the location of the given remote participant for spatial audio purposes.
	 *
	 * This is only applicable when the spatial audio style of the conference is set to "Individual".
	 *
	 * Calling this function with the local participant ID has no effect. Use Set Local Player Location instead.
	 *
	 * @param ParticipantID - The ID of the remote participant.
	 * @param Location - The location of the remote participant.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetRemotePlayerLocation(const FString& ParticipantID, const FVector& Location);

	/** Sets what to log in the Dolby.io C++ SDK. The logs will be saved to the default project log directory (likely
	 * Saved/Logs).
	 *
	 * This function should be called before the first call to Set Token if the user needs logs about the plugin's
	 * operation. Calling this function more than once has no effect.
	 *
	 * @param SdkLogLevel - Log level for SDK logs.
	 * @param MediaLogLevel - Log level for Media Engine logs.
	 * @param DvcLogLevel - Log level for DVC logs.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetLogSettings(EDolbyIOLogLevel SdkLogLevel = EDolbyIOLogLevel::Info,
	                    EDolbyIOLogLevel MediaLogLevel = EDolbyIOLogLevel::Info,
	                    EDolbyIOLogLevel DvcLogLevel = EDolbyIOLogLevel::Info);

	/** Gets a list of all available audio input devices.
	 *
	 * Triggers On Audio Input Devices Received if successful.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void GetAudioInputDevices();

	/** Gets a list of all available audio output devices.
	 *
	 * Triggers On Audio Output Devices Received if successful.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void GetAudioOutputDevices();

	/** Gets the current audio input device.
	 *
	 * Triggers On Current Audio Input Device Received if successful.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void GetCurrentAudioInputDevice();

	/** Gets the current audio output device.
	 *
	 * Triggers On Current Audio Output Device Received if successful.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void GetCurrentAudioOutputDevice();

	/** Sets the audio input device.
	 *
	 * Triggers On Current Audio Input Device Changed if successful.
	 *
	 * @param NativeID - The ID of the device to use.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetAudioInputDevice(const FString& NativeID);

	/** Sets the audio output device.
	 *
	 * Triggers On Current Audio Output Device Changed if successful.
	 *
	 * @param NativeID - The ID of the device to use.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetAudioOutputDevice(const FString& NativeID);

	/** Gets a list of all available video devices.
	 *
	 * Triggers On Video Devices Received if successful.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void GetVideoDevices();

	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnTokenNeededDelegate OnTokenNeeded;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnInitializedDelegate OnInitialized;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnConnectedDelegate OnConnected;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnDisconnectedDelegate OnDisconnected;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnParticipantAddedDelegate OnParticipantAdded;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnParticipantUpdatedDelegate OnParticipantUpdated;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnVideoTrackAddedDelegate OnVideoTrackAdded;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnVideoTrackRemovedDelegate OnVideoTrackRemoved;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnVideoTrackEnabledDelegate OnVideoTrackEnabled;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnVideoTrackDisabledDelegate OnVideoTrackDisabled;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnVideoEnabledDelegate OnVideoEnabled;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnVideoDisabledDelegate OnVideoDisabled;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnScreenshareStartedDelegate OnScreenshareStarted;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnScreenshareStoppedDelegate OnScreenshareStopped;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnActiveSpeakersChangedDelegate OnActiveSpeakersChanged;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnAudioLevelsChangedDelegate OnAudioLevelsChanged;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnScreenshareSourcesReceivedDelegate OnScreenshareSourcesReceived;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnAudioInputDevicesReceivedDelegate OnAudioInputDevicesReceived;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnAudioOutputDevicesReceivedDelegate OnAudioOutputDevicesReceived;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnCurrentAudioInputDeviceReceivedDelegate OnCurrentAudioInputDeviceReceived;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnCurrentAudioOutputDeviceReceivedDelegate OnCurrentAudioOutputDeviceReceived;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnVideoDevicesReceivedDelegate OnVideoDevicesReceived;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnCurrentAudioInputDeviceChangedDelegate OnCurrentAudioInputDeviceChanged;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnCurrentAudioOutputDeviceChangedDelegate OnCurrentAudioOutputDeviceChanged;

private:
	void Initialize(FSubsystemCollectionBase&) override;
	void Deinitialize() override;

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

	template <class TDelegate, class... TArgs> void BroadcastEvent(TDelegate&, TArgs&&...);

	dolbyio::comms::conference_status ConferenceStatus;
	FString LocalParticipantID;
	FString ConferenceID;
	EDolbyIOConnectionMode ConnectionMode;
	EDolbyIOSpatialAudioStyle SpatialAudioStyle;
	TMap<FString, TArray<FDolbyIOVideoTrack>> BufferedVideoTracks;
	TMap<FString, FDolbyIOParticipantInfo> RemoteParticipants;
	FCriticalSection RemoteParticipantsLock;

	TMap<FString, std::shared_ptr<DolbyIO::FVideoSink>> VideoSinks;
	std::shared_ptr<DolbyIO::FVideoFrameHandler> LocalCameraFrameHandler;
	std::shared_ptr<DolbyIO::FVideoFrameHandler> LocalScreenshareFrameHandler;
	TSharedPtr<dolbyio::comms::sdk> Sdk;
	TSharedPtr<dolbyio::comms::refresh_token> RefreshTokenCb;

	float SpatialEnvironmentScale = 1.0f;

	bool bIsInputMuted = false;
	bool bIsOutputMuted = false;
	bool bIsVideoEnabled = false;

	FTimerHandle LocationTimerHandle;
	FTimerHandle RotationTimerHandle;

	class FErrorHandler final
	{
	public:
		FErrorHandler(UDolbyIOSubsystem& DolbyIOSubsystem, int Line);

		void operator()(std::exception_ptr&& ExcPtr) const;
		void HandleError() const;

	private:
		void HandleError(TFunction<void()> Callee) const;
		void LogException(const FString& Type, const FString& What) const;

		UDolbyIOSubsystem& DolbyIOSubsystem;
		int Line;
	};
};
