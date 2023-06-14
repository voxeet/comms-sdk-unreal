// Copyright 2023 Dolby Laboratories

#pragma once

#include "Components/ActorComponent.h"

#include "DolbyIODevices.h"
#include "DolbyIOParticipantInfo.h"
#include "DolbyIOScreenshareSource.h"
#include "DolbyIOVideoTrack.h"

#include "DolbyIOObserver.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FObserverOnTokenNeededDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FObserverOnInitializedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FObserverOnConnectedDelegate, const FString&, LocalParticipantID,
                                             const FString&, ConferenceID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FObserverOnDisconnectedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FObserverOnParticipantAddedDelegate, const EDolbyIOParticipantStatus,
                                             Status, const FDolbyIOParticipantInfo&, ParticipantInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FObserverOnParticipantUpdatedDelegate, const EDolbyIOParticipantStatus,
                                             Status, const FDolbyIOParticipantInfo&, ParticipantInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObserverOnVideoTrackAddedDelegate, const FDolbyIOVideoTrack&, VideoTrack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObserverOnVideoTrackRemovedDelegate, const FDolbyIOVideoTrack&,
                                            VideoTrack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObserverOnVideoTrackEnabledDelegate, const FDolbyIOVideoTrack&,
                                            VideoTrack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObserverOnVideoTrackDisabledDelegate, const FDolbyIOVideoTrack&,
                                            VideoTrack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObserverOnVideoEnabledDelegate, const FString&, VideoTrackID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObserverOnVideoDisabledDelegate, const FString&, VideoTrackID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObserverOnScreenshareStartedDelegate, const FString&, VideoTrackID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObserverOnScreenshareStoppedDelegate, const FString&, VideoTrackID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObserverOnActiveSpeakersChangedDelegate, const TArray<FString>&,
                                            ActiveSpeakers);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FObserverOnAudioLevelsChangedDelegate, const TArray<FString>&,
                                             ActiveSpeakers, const TArray<float>&, AudioLevels);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObserverOnScreenshareSourcesReceivedDelegate,
                                            const TArray<FDolbyIOScreenshareSource>&, Sources);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObserverOnAudioInputDevicesReceivedDelegate,
                                            const TArray<FDolbyIOAudioDevice>&, Devices);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObserverOnAudioOutputDevicesReceivedDelegate,
                                            const TArray<FDolbyIOAudioDevice>&, Devices);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FObserverOnCurrentAudioInputDeviceReceivedDelegate, bool, IsNone,
                                             const FDolbyIOAudioDevice&, OptionalDevice);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FObserverOnCurrentAudioOutputDeviceReceivedDelegate, bool, IsNone,
                                             const FDolbyIOAudioDevice&, OptionalDevice);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObserverOnVideoDevicesReceivedDelegate, const TArray<FDolbyIOVideoDevice>&,
                                            Devices);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FObserverOnCurrentAudioInputDeviceChangedDelegate, bool, IsNone,
                                             const FDolbyIOAudioDevice&, OptionalDevice);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FObserverOnCurrentAudioOutputDeviceChangedDelegate, bool, IsNone,
                                             const FDolbyIOAudioDevice&, OptionalDevice);

UCLASS(ClassGroup = "Dolby.io Comms",
       Meta = (BlueprintSpawnableComponent, DisplayName = "Dolby.io Observer",
               ToolTip = "Component to use for easy handling of Dolby.io subsystem events."))
class DOLBYIO_API UDolbyIOObserver : public UActorComponent
{
	GENERATED_BODY()

public:
	UDolbyIOObserver();

	/** Triggered when an initial or refreshed client access token
	 * (https://docs.dolby.io/communications-apis/docs/overview-developer-tools#client-access-token) is needed, which
	 * happens when the Dolby.io Subsystem is initialized or when a refresh token is requested. After receiving this
	 * event, obtain a token for your Dolby.io application and call the Dolby.io Set Token function.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnTokenNeededDelegate OnTokenNeeded;

	/** Triggered when the plugin is successfully initialized after calling the Set Token function. After receiving this
	 * event, the plugin is ready for use. You can now, for example, call the Dolby.io Connect function.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnInitializedDelegate OnInitialized;

	/** Triggered when the client is successfully connected to the conference after calling the Connect function. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnConnectedDelegate OnConnected;

	/** Triggered when the client is disconnected from the conference by any means; in particular, by the Disconnect
	 * function.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnDisconnectedDelegate OnDisconnected;

	/** Triggered when a remote participant is added to the conference. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnParticipantAddedDelegate OnParticipantAdded;

	/** Triggered when a remote participant's status is updated. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnParticipantUpdatedDelegate OnParticipantUpdated;

	/** Triggered when a video track is added. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnVideoTrackAddedDelegate OnVideoTrackAdded;

	/** Triggered when a video track is removed. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnVideoTrackRemovedDelegate OnVideoTrackRemoved;

	/** Triggered when a video track is enabled as a result of the video forwarding strategy. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnVideoTrackEnabledDelegate OnVideoTrackEnabled;

	/** Triggered when a video track is disabled as a result of the video forwarding strategy. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnVideoTrackDisabledDelegate OnVideoTrackDisabled;

	/** Triggered when local video is enabled. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnVideoEnabledDelegate OnVideoEnabled;

	/** Triggered when local video is disabled. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnVideoDisabledDelegate OnVideoDisabled;

	/** Triggered when screenshare is started. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnScreenshareStartedDelegate OnScreenshareStarted;

	/** Triggered when screenshare is stopped. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnScreenshareStoppedDelegate OnScreenshareStopped;

	/** Triggered when participants start or stop speaking. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnActiveSpeakersChangedDelegate OnActiveSpeakersChanged;

	/** Triggered roughly every 500ms. The event provides two arrays: an array of IDs of the current speakers and an
	 * array of floating point numbers representing each participant's audio level. The order of Audio Levels
	 * corresponds to the order of Active Speakers. A value of 0.0 represents silence and a value of 1.0 represents the
	 * maximum volume. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnAudioLevelsChangedDelegate OnAudioLevelsChanged;

	/** Triggered when screen share sources are received as a result of calling Get Screenshare Sources. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnScreenshareSourcesReceivedDelegate OnScreenshareSourcesReceived;

	/** Triggered when audio input devices are received as a result of calling Get Audio Input Devices. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnAudioInputDevicesReceivedDelegate OnAudioInputDevicesReceived;

	/** Triggered when audio output devices are received as a result of calling Get Audio Output Devices. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnAudioOutputDevicesReceivedDelegate OnAudioOutputDevicesReceived;

	/** Triggered when the current audio input device is received as a result of calling Get Current Audio Input Device.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnCurrentAudioInputDeviceReceivedDelegate OnCurrentAudioInputDeviceReceived;

	/** Triggered when the current audio output device is received as a result of calling Get Current Audio Output
	 * Device.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnCurrentAudioOutputDeviceReceivedDelegate OnCurrentAudioOutputDeviceReceived;

	/** Triggered when video devices are received as a result of calling Get Video Devices. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnVideoDevicesReceivedDelegate OnVideoDevicesReceived;

	/** Triggered when the current audio input device is changed by the user or as a result of calling
	 * Set Current Audio Input Device. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnCurrentAudioInputDeviceChangedDelegate OnCurrentAudioInputDeviceChanged;

	/** Triggered when the current audio output device is changed by the user or as a result of calling
	 * Set Current Audio Output Device. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnCurrentAudioOutputDeviceChangedDelegate OnCurrentAudioOutputDeviceChanged;

private:
	void InitializeComponent() override;

	UFUNCTION()
	void FwdOnTokenNeeded();

	UFUNCTION()
	void FwdOnInitialized();

	UFUNCTION()
	void FwdOnConnected(const FString& LocalParticipantID, const FString& ConferenceID);

	UFUNCTION()
	void FwdOnDisconnected();

	UFUNCTION()
	void FwdOnParticipantAdded(const EDolbyIOParticipantStatus Status, const FDolbyIOParticipantInfo& ParticipantInfo);

	UFUNCTION()
	void FwdOnParticipantUpdated(const EDolbyIOParticipantStatus Status,
	                             const FDolbyIOParticipantInfo& ParticipantInfo);

	UFUNCTION()
	void FwdOnVideoTrackAdded(const FDolbyIOVideoTrack& VideoTrack);

	UFUNCTION()
	void FwdOnVideoTrackRemoved(const FDolbyIOVideoTrack& VideoTrack);

	UFUNCTION()
	void FwdOnVideoTrackEnabled(const FDolbyIOVideoTrack& VideoTrack);

	UFUNCTION()
	void FwdOnVideoTrackDisabled(const FDolbyIOVideoTrack& VideoTrack);

	UFUNCTION()
	void FwdOnVideoEnabled(const FString& VideoTrackID);

	UFUNCTION()
	void FwdOnVideoDisabled(const FString& VideoTrackID);

	UFUNCTION()
	void FwdOnScreenshareStarted(const FString& VideoTrackID);

	UFUNCTION()
	void FwdOnScreenshareStopped(const FString& VideoTrackID);

	UFUNCTION()
	void FwdOnActiveSpeakersChanged(const TArray<FString>& ActiveSpeakers);

	UFUNCTION()
	void FwdOnAudioLevelsChanged(const TArray<FString>& ActiveSpeakers, const TArray<float>& AudioLevels);

	UFUNCTION()
	void FwdOnScreenshareSourcesReceived(const TArray<FDolbyIOScreenshareSource>& Sources);

	UFUNCTION()
	void FwdOnAudioInputDevicesReceived(const TArray<FDolbyIOAudioDevice>& Devices);

	UFUNCTION()
	void FwdOnAudioOutputDevicesReceived(const TArray<FDolbyIOAudioDevice>& Devices);

	UFUNCTION()
	void FwdOnCurrentAudioInputDeviceReceived(bool IsNone, const FDolbyIOAudioDevice& OptionalDevice);

	UFUNCTION()
	void FwdOnCurrentAudioOutputDeviceReceived(bool IsNone, const FDolbyIOAudioDevice& OptionalDevice);

	UFUNCTION()
	void FwdOnVideoDevicesReceived(const TArray<FDolbyIOVideoDevice>& Devices);

	UFUNCTION()
	void FwdOnCurrentAudioInputDeviceChanged(bool IsNone, const FDolbyIOAudioDevice& OptionalDevice);

	UFUNCTION()
	void FwdOnCurrentAudioOutputDeviceChanged(bool IsNone, const FDolbyIOAudioDevice& OptionalDevice);

	template <class TDelegate, class... TArgs> void BroadcastEvent(TDelegate&, TArgs&&...);
};
