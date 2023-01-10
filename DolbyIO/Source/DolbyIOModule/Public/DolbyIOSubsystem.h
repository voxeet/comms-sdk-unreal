// Copyright 2022 Dolby Laboratories

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"

#include "DolbyIOParticipantInfo.h"

#include "Delegates/Delegate.h"
#include "Engine/EngineTypes.h"
#include "HAL/ThreadSafeBool.h"

#include "DolbyIOSubsystem.generated.h"

namespace DolbyIO
{
	class FAuthenticator;
	class FSdkAccess;
}

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSubsystemOnTokenNeededDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSubsystemOnInitializedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubsystemOnConnectedDelegate, const FString&, LocalParticipantID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSubsystemOnDisconnectedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubsystemOnParticipantAddedDelegate, const FDolbyIOParticipantInfo&,
                                            ParticipantInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubsystemOnParticipantLeftDelegate, const FDolbyIOParticipantInfo&,
                                            ParticipantInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubsystemOnActiveSpeakersChangedDelegate, const TArray<FString>&,
                                            ActiveSpeakers);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSubsystemOnAudioLevelsChangedDelegate, const TArray<FString>&,
                                             ActiveSpeakers, const TArray<float>&, AudioLevels);

/** The Dolby.io Virtual World plugin game instance subsystem. */
UCLASS(DisplayName = "Dolby.io Subsystem")
class DOLBYIOMODULE_API UDolbyIOSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/*
	 *
	 * Functions for controlling the plugin's behavior, callable from Blueprints.
	 *
	 */

	/** Initializes or refreshes the client access token. The function takes the token as a parameter and initializes
	 * the plugin unless already initialized. Successful initialization triggers the [On Initialized](#on-initialized)
	 * event.
	 *
	 * For quick testing, you can manually obtain a token from the [Dolby.io dashboard](https://dashboard.dolby.io/) and
	 * paste it directly into the node.
	 *
	 * You may use the [Set Token Using Key and Secret](#set-token-using-key-and-secret) function instead for
	 * convenience during onboarding.
	 *
	 * Example:
	 * <img src="https://files.readme.io/e44088b-on_token_needed.PNG">
	 * @param Token - The client access token.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io")
	void SetToken(const FString& Token);

	/** Initializes or refreshes the client access token. The function is similar to [Set Token](#set-token), except it
	 * takes an app key and secret as parameters and automatically generates the token.
	 *
	 * For convenience during early development and prototyping, this function is provided to acquire the client access
	 * token directly from within the application. However, please note **we do not recommend** using this mechanism in
	 * the production software for [security best
	 * practices](https://docs.dolby.io/communications-apis/docs/guides-client-authentication). App secret needs to be
	 * protected and not included in the application.
	 *
	 * Example:
	 * <img src="https://files.readme.io/f38e604-set_token_using_key_and_secret.PNG">
	 * @param AppKey - The app key.
	 * @param AppSecret - The app secret.
	 * @param TokenExpirationTimeInSeconds - The token's expiration time (in seconds).
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io")
	void SetTokenUsingKeyAndSecret(const FString& AppKey, const FString& AppSecret,
	                               int TokenExpirationTimeInSeconds = 3600);

	/** Connects to a conference. The method triggers [On Connected](#on-connected) if successful.
	 * @param ConferenceName - The conference name. Must not be empty.
	 * @param UserName - The name of the participant.
	 * @param ExternalID - The external unique identifier that the customer's application can add to the participant
	 * while opening a session. If a participant uses the same external ID in conferences, the participant's ID also
	 * remains the same across all sessions.
	 * @param AvatarURL - The URL of the participant's avatar.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io")
	void Connect(const FString& ConferenceName = "unreal", const FString& UserName = "", const FString& ExternalID = "",
	             const FString& AvatarURL = "");

	/** Connects to a demo conference, which automatically brings in 3 invisible bots into the conference as a quick way
	 * to validate the connection to the service with audio functionality. One of the bots is placed to the left of
	 * point {0, 0, 0} in the level, one is placed to the right and one circles around that point. The method triggers
	 * [On Connected](#on-connected) if successful.
	 *
	 * Example:
	 * <img src="https://files.readme.io/db8d689-connect_to_demo_conference.PNG">
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io")
	void ConnectToDemoConference();

	/** Disconnects from the current conference. Triggers [On Disconnected](#on-disconnected) when complete.
	 *
	 * Example:
	 * <img src="https://files.readme.io/61921d2-disconnect.PNG">
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io")
	void Disconnect();

	/** Sets the spatial environment scale. The larger the scale, the longer the distance at which the spatial audio
	 * attenuates. To get the best experience, the scale should be set separately for each level. The default value of
	 * "1.0" means that audio will fall completely silent at a distance of 10000 units (10000 cm/100 m).
	 * @param SpatialEnvironmentScale - The scale as a floating point number.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io")
	void SetSpatialEnvironmentScale(float SpatialEnvironmentScale = 1.0f);

	/** Mutes audio input.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io")
	void MuteInput();

	/** Unmutes audio input.
	 *
	 * Example:
	 * <img src=https://files.readme.io/9c112d7-mute_input.PNG">
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io")
	void UnmuteInput();

	/** Mutes audio output.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io")
	void MuteOutput();

	/** Unmutes audio output.
	 *
	 * Example:
	 * <img src="https://files.readme.io/140e9f7-mute_output.PNG">
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io")
	void UnmuteOutput();

	/** Gets audio levels for all speaking participants. Triggers [On Audio Levels Changed](#on-audio-levels-changed) if
	 * successful.
	 *
	 * Example:
	 * <img src="https://files.readme.io/0786858-get_audio_levels.PNG">
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io")
	void GetAudioLevels();

	/** Updates the position and rotation of the listener for spatial audio purposes. Calling this function even once
	 * disables the default behavior, which is to automatically use the location and rotation of the first player
	 * controller.
	 *
	 * Example:
	 * <img src="https://files.readme.io/1bfa225-update_view_point.PNG">
	 * @param Position - The location of the listener.
	 * @param Rotation - The rotation of the listener.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io")
	void UpdateViewPoint(const FVector& Position, const FRotator& Rotation);

	/** Triggered when an initial or refreshed [client access
	 * token](https://docs.dolby.io/communications-apis/docs/overview-developer-tools#client-access-token) is needed,
	 * which happens when the game starts or when a refresh token is requested. After receiving this event, obtain a
	 * token for your Dolby.io application and call the [Set Token](#set-token) function.
	 *
	 * Example:
	 * <img src="https://files.readme.io/e44088b-on_token_needed.PNG">
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io")
	FSubsystemOnTokenNeededDelegate OnTokenNeeded;

	/** Triggered when the plugin is successfully initialized after calling the [Set Token](#set-token) function.
	 * After receiving this event, the plugin is ready for use. You can now, for example, call this Blueprint's
	 * [Connect](#connect) function. Once connected, the [On Connected](#on-connected) event will trigger.
	 *
	 * Example:
	 * <img src="https://files.readme.io/124a74c-on_initialized.PNG">
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io")
	FSubsystemOnInitializedDelegate OnInitialized;

	/** Triggered when the client is successfully connected to the conference after calling the [Connect](#connect)
	 * function.
	 *
	 * Example:
	 * <img src="https://files.readme.io/d6744e0-on_connected.PNG">
	 * @param LocalParticipant - A string holding the ID of the local participant.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io")
	FSubsystemOnConnectedDelegate OnConnected;

	/** Triggered when the client is disconnected from the conference by any means; in particular, by the
	 * [Disconnect](#disconnect) function.
	 *
	 * Example:
	 * <img src="https://files.readme.io/8322383-on_disconnected.PNG">
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io")
	FSubsystemOnDisconnectedDelegate OnDisconnected;

	/** Triggered when remote participants are added to or removed from the conference.
	 *
	 * Example:
	 * <img src="https://files.readme.io/9b036e5-on_participant_added.PNG">
	 * @param RemoteParticipants - A set of strings holding the IDs of remote participants.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io")
	FSubsystemOnParticipantAddedDelegate OnParticipantAdded;

	/** Triggered when a remote participant leaves the conference.
	 *
	 * Example:
	 * <img src="https://files.readme.io/f4af244-on_participant_left.PNG">
	 * @param ParticipantInfo - Contains the current status of a conference participant and information whether the
	 * participant's audio is enabled.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io")
	FSubsystemOnParticipantLeftDelegate OnParticipantLeft;

	/** Triggered when participants start or stop speaking.
	 *
	 * Example:
	 * <img src="https://files.readme.io/45fb4dd-on_active_speakers_changed.PNG">
	 * @param ActiveSpeakers - The IDs of the current active speakers.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io")
	FSubsystemOnActiveSpeakersChangedDelegate OnActiveSpeakersChanged;

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
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io")
	FSubsystemOnAudioLevelsChangedDelegate OnAudioLevelsChanged;

private:
	void UpdateViewPointUsingFirstPlayer();

	// UGameInstanceSubsystem
	void Initialize(FSubsystemCollectionBase&) override;

	TSharedPtr<DolbyIO::FSdkAccess> CppSdk;
	TSharedPtr<DolbyIO::FAuthenticator> Authenticator;

	class UGameInstance* GameInstance;
	FTimerHandle SpatialUpdateTimerHandle;
};
