// Copyright 2022 Dolby Laboratories

#pragma once

#include "Components/ActorComponent.h"

#include "DolbyIOParticipantInfo.h"

#include "Delegates/Delegate.h"

#include "DolbyIOObserver.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FObserverOnTokenNeededDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FObserverOnInitializedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObserverOnConnectedDelegate, const FString&, LocalParticipantID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FObserverOnDisconnectedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObserverOnParticipantAddedDelegate, const FDolbyIOParticipantInfo&,
                                            ParticipantInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObserverOnParticipantLeftDelegate, const FDolbyIOParticipantInfo&,
                                            ParticipantInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObserverOnActiveSpeakersChangedDelegate, const TArray<FString>&,
                                            ActiveSpeakers);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FObserverOnAudioLevelsChangedDelegate, const TArray<FString>&,
                                             ActiveSpeakers, const TArray<float>&, AudioLevels);

UCLASS(ClassGroup = "Dolby.io Comms", DisplayName = "Dolby.io Observer", Meta = (BlueprintSpawnableComponent))
class DOLBYIO_API UDolbyIOObserver : public UActorComponent
{
	GENERATED_BODY()

public:
	UDolbyIOObserver();

	/** Triggered when an initial or refreshed [client access
	 * token](https://docs.dolby.io/communications-apis/docs/overview-developer-tools#client-access-token) is needed,
	 * which happens when the game starts or when a refresh token is requested. After receiving this event, obtain a
	 * token for your Dolby.io application and call the [Set Token](#set-token) function.
	 *
	 * Example:
	 * <img src="https://files.readme.io/e44088b-on_token_needed.PNG">
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnTokenNeededDelegate OnTokenNeeded;

	/** Triggered when the plugin is successfully initialized after calling the [Set Token](#set-token) function.
	 * After receiving this event, the plugin is ready for use. You can now, for example, call this Blueprint's
	 * [Connect](#connect) function. Once connected, the [On Connected](#on-connected) event will trigger.
	 *
	 * Example:
	 * <img src="https://files.readme.io/124a74c-on_initialized.PNG">
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnInitializedDelegate OnInitialized;

	/** Triggered when the client is successfully connected to the conference after calling the [Connect](#connect)
	 * function.
	 *
	 * Example:
	 * <img src="https://files.readme.io/d6744e0-on_connected.PNG">
	 * @param LocalParticipant - A string holding the ID of the local participant.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnConnectedDelegate OnConnected;

	/** Triggered when the client is disconnected from the conference by any means; in particular, by the
	 * [Disconnect](#disconnect) function.
	 *
	 * Example:
	 * <img src="https://files.readme.io/8322383-on_disconnected.PNG">
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnDisconnectedDelegate OnDisconnected;

	/** Triggered when remote participants are added to or removed from the conference.
	 *
	 * Example:
	 * <img src="https://files.readme.io/9b036e5-on_participant_added.PNG">
	 * @param RemoteParticipants - A set of strings holding the IDs of remote participants.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnParticipantAddedDelegate OnParticipantAdded;

	/** Triggered when a remote participant leaves the conference.
	 *
	 * Example:
	 * <img src="https://files.readme.io/f4af244-on_participant_left.PNG">
	 * @param ParticipantInfo - Contains the current status of a conference participant and information whether the
	 * participant's audio is enabled.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnParticipantLeftDelegate OnParticipantLeft;

	/** Triggered when participants start or stop speaking.
	 *
	 * Example:
	 * <img src="https://files.readme.io/45fb4dd-on_active_speakers_changed.PNG">
	 * @param ActiveSpeakers - The IDs of the current active speakers.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnActiveSpeakersChangedDelegate OnActiveSpeakersChanged;

	/** Triggered when there are new audio levels available after calling the [Get Audio Levels](#get-audio-levels)
	 * function.
	 *
	 * Example:
	 * <img src="https://files.readme.io/fdb5789-on_audio_levels_changed.PNG">
	 * @param ActiveSpeakers - The IDs of the current active speakers.
	 * @param AudioLevels - A floating point number representing each participant's audio level. The order of levels
	 * corresponds to the order of ActiveSpeakers. A value of 0.0 represents silence and a value of 1.0 represents the
	 * maximum volume.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnAudioLevelsChangedDelegate OnAudioLevelsChanged;

private:
	void InitializeComponent() override;

	UFUNCTION()
	void FwdOnTokenNeeded();

	UFUNCTION()
	void FwdOnInitialized();

	UFUNCTION()
	void FwdOnConnected(const FString& LocalParticipantID);

	UFUNCTION()
	void FwdOnDisconnected();

	UFUNCTION()
	void FwdOnParticipantAdded(const FDolbyIOParticipantInfo& ParticipantInfo);

	UFUNCTION()
	void FwdOnParticipantLeft(const FDolbyIOParticipantInfo& ParticipantInfo);

	UFUNCTION()
	void FwdOnActiveSpeakersChanged(const TArray<FString>& ActiveSpeakers);

	UFUNCTION()
	void FwdOnAudioLevelsChanged(const TArray<FString>& ActiveSpeakers, const TArray<float>& AudioLevels);

	template <class TDelegate, class... TArgs> void BroadcastEvent(TDelegate&, TArgs&&...);
};
