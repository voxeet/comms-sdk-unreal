// Copyright 2022 Dolby Laboratories

#pragma once

#include "DolbyIO/SdkEventObserver.h"
#include "DolbyIOParticipantInfo.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "Engine/EngineTypes.h"
#include "HAL/ThreadSafeBool.h"

#include "DolbyIO.generated.h"

namespace DolbyIO
{
	class FAuthenticator;
	class FSdkAccess;
}

/** The Dolby.io Virtual World plugin game instance subsystem. */
UCLASS(Abstract, Blueprintable)
class DOLBYIOMODULE_API UDolbyIO : public UGameInstanceSubsystem, public DolbyIO::ISdkEventObserver
{
	GENERATED_BODY()

public:
	/*
	 *
	 * Functions for controlling the plugin's behavior, callable from Blueprints.
	 *
	 */

	/** Initializes or refreshes the client access token. The function takes the token as a parameter and initializes the plugin unless already initialized. Successful initialization triggers the [On Initialized](#on-initialized) event.
	 *
	 * For quick testing, you can manually obtain a token from the [Dolby.io dashboard](https://dashboard.dolby.io/) and paste it directly into the node.
	 *
	 * You may use the [Set Token Using Key and Secret](#set-token-using-key-and-secret) function instead for convenience during onboarding.
	 *
	 * Example:
	 * ![example](./../../../../Images/on_token_needed.PNG)
	 * @param Token - The client access token.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void SetToken(const FString& Token);

	/** Initializes or refreshes the client access token. The function is similar to [Set Token](#set-token), except it takes an app key and secret as parameters and automatically generates the token.
	 *
	 * For convenience during early development and prototyping, this function is provided to acquire the client access token directly from within the application. However, please note **we do not recommend** using this mechanism in the production software for [security best practices](https://docs.dolby.io/communications-apis/docs/guides-client-authentication). App secret needs to be protected and not included in the application.
	 *
	 * Example:
	 * ![example](./../../../../Images/set_token_using_key_and_secret.PNG)
	 * @param AppKey - The app key.
	 * @param AppSecret - The app secret.
	 * @param TokenExpirationTimeInSeconds - The token's expiration time (in seconds).
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void SetTokenUsingKeyAndSecret(const FString& AppKey, const FString& AppSecret,
	                               int TokenExpirationTimeInSeconds = 3600);

	/** Connects to a conference. Triggers OnConnected if successful.
	 * @param ConferenceName - Conference name. Must not be empty.
	 * @param UserName - The name of the participant.
	 * @param ExternalID - The external unique identifier that the customer's application can add to the participant
	 * while opening a session. If a participant uses the same external ID in conferences, the participan's ID also
	 * remains the same across all sessions.
	 * @param AvatarURL - The URL of the participant's avatar.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void Connect(const FString& ConferenceName = "unreal", const FString& UserName = "", const FString& ExternalID = "",
	             const FString& AvatarURL = "");

	/** Connects to a demo conference, which automatically brings in 3 invisible bots into the conference as a quick way
	 * to validate the connection to the service with audio functionality. One of the bots is placed to the left of
	 * point {0, 0, 0} in the level, one is placed to the right and one circles around that point. Triggers OnConnected
	 * if successful.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void ConnectToDemoConference();

	/** Disconnects from the current conference. Triggers [On Disconnected](#on-disconnected) when complete.
	 *
	 * Example:
	 * ![example](./../../../../Images/disconnect.PNG)
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void Disconnect();

	/** Mutes audio input. The method has no effect unless the client is connected to the conference.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void MuteInput();

	/** Unmutes audio input. The method has no effect unless the client is connected to the conference.
	 *
	 * Example:
	 * ![example](./../../../../Images/mute_input.PNG)
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void UnmuteInput();

	/** Mutes audio output. The method has no effect unless the client is connected to the conference.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void MuteOutput();

	/** Unmutes audio output. The method has no effect unless the client is connected to the conference.
	 *
	 * Example:
	 * ![example](./../../../../Images/mute_output.PNG)
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void UnmuteOutput();

	/** Gets audio levels for all speaking participants. Triggers [On Audio Levels Changed](#on-audio-levels-changed) if successful.
	 *
	 * Example:
	 * ![example](./../../../../Images/get_audio_levels.PNG)
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void GetAudioLevels();

	/** Updates the position and rotation of the listener for spatial audio purposes. Calling this function even once disables the default behavior, which is to automatically use the location and rotation of the first player controller.
	 *
	 * Example:
	 * ![example](./../../../../Images/update_view_point.PNG)
	 * @param Position - The location of the listener.
	 * @param Rotation - The rotation of the listener.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void UpdateViewPoint(const FVector& Position, const FRotator& Rotation);

	/*
	 *
	 * Events triggered from C++, implementable in Blueprints.
	 *
	 */

	/** Triggered when an initial or refreshed [client access token](https://docs.dolby.io/communications-apis/docs/overview-developer-tools#client-access-token) is needed, which happens when the game starts or when a refresh token is requested.
	 * After receiving this event, obtain a token for your Dolby.io application and call the [Set Token](#set-token) function.
	 *
	 * Example:
	 * ![example](./../../../../Images/on_token_needed.PNG)
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnTokenNeeded();

	/** Triggered when the plugin is successfully initialized after calling the [Set Token](#set-token) function.
	 * After receiving this event, the plugin is ready for use. You can now, for example, call this Blueprint's [Connect](#connect) function. Once connected, the [On Connected](#on-connected) event will trigger.
	 *
	 * Example:
	 * ![example](./../../../../Images/on_initialized.PNG)
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnInitialized();

	/** Triggered when the client is successfully connected to the conference after calling the [Connect](#connect) function.
	 *
	 * Example:
	 * ![example](./../../../../Images/on_connected.PNG)
	 * @param LocalParticipant - A string holding the ID of the local participant.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnConnected(const FString& LocalParticipantID);

	/** Triggered when the client is disconnected from the conference by any means; in particular, by the [Disconnect](#disconnect) function.
	 *
	 * Example:
	 * ![example](./../../../../Images/on_disconnected.PNG)
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnDisconnected();

	/** Triggered when remote participants are added to or removed from the conference.
	 *
	 * Example:
	 * ![example](./../../../../Images/on_remote_participants_changed.PNG)
	 * @param RemoteParticipants - A set of strings holding the IDs of remote participants.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnParticipantAdded(const FDolbyIOParticipantInfo& ParticipantInfo);

	/** Triggered when a remote participant leaves the conference.
	 * @param ParticipantInfo - Contains the current status of a conference participant and information whether the
	 * participant's audio is enabled.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnParticipantLeft(const FDolbyIOParticipantInfo& ParticipantInfo);

	/** Triggered when participants start or stop speaking.
	 *
	 * Example:
	 * ![example](./../../../../Images/on_active_speakers_changed.PNG)
	 * @param ActiveSpeakers - IDs of the current active speakers.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnActiveSpeakersChanged(const TArray<FString>& ActiveSpeakers);

	/** Triggered when there are new audio levels available after calling the [Get Audio Levels](#get-audio-levels) function.
	 *
	 * Example:
	 * ![example](./../../../../Images/on_audio_levels_changed.PNG)
	 * @param AudioLevels - A string-to-float mapping of participant IDs to their audio levels. A value of 0.0 represents silence and a value of 1.0 represents the maximum volume.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnAudioLevelsChanged(const TMap<FString, float>& AudioLevels);

private:
	void UpdateViewPointUsingFirstPlayer();

	// UGameInstanceSubsystem
	void Initialize(FSubsystemCollectionBase&) override;
	void Deinitialize() override;

	// ISdkEventObserver
	void OnTokenNeededEvent() override;
	void OnInitializedEvent() override;
	void OnConnectedEvent(const DolbyIO::FParticipantID&) override;
	void OnDisconnectedEvent() override;
	void OnParticipantAddedEvent(const FDolbyIOParticipantInfo&) override;
	void OnParticipantLeftEvent(const FDolbyIOParticipantInfo&) override;
	void OnActiveSpeakersChangedEvent(const DolbyIO::FParticipantIDs&) override;
	void OnAudioLevelsChangedEvent(const DolbyIO::FAudioLevels&) override;

	TSharedPtr<DolbyIO::FSdkAccess> CppSdk;
	TSharedPtr<DolbyIO::FAuthenticator> Authenticator;

	class UGameInstance* GameInstance;
	FTimerHandle SpatialUpdateTimerHandle;
	FThreadSafeBool bIsAlive = true;
};
