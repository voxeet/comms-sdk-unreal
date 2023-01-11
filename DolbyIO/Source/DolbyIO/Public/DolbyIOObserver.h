// Copyright 2023 Dolby Laboratories

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
	 * happens when the game starts or when a refresh token is requested. After receiving this event, obtain a token for
	 * your Dolby.io application and call the Dolby.io Subsystem's Set Token function.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnTokenNeededDelegate OnTokenNeeded;

	/** Triggered when the plugin is successfully initialized after calling the Set Token function. After receiving this
	 * event, the plugin is ready for use. You can now, for example, call the Dolby.io Subsystem's Connect function.
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

	/** Triggered when a remote participant leaves the conference. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnParticipantLeftDelegate OnParticipantLeft;

	/** Triggered when participants start or stop speaking. */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FObserverOnActiveSpeakersChangedDelegate OnActiveSpeakersChanged;

	/** Triggered when there are new audio levels available after calling the Get Audio Levels function. */
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
