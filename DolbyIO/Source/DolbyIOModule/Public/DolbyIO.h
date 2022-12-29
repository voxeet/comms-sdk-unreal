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

	/** Initializes or refreshes the client access token. Initializes the plugin unless already initialized. Triggers
	 * OnInitialized if initialization was successful.
	 * @param Token - Client access token.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void SetToken(const FString& Token);

	/** DO NOT USE IN PRODUCTION. Automatically obtains a client access token using an app key and secret and calls
	 * SetToken.
	 * @param AppKey - App key.
	 * @param AppSecret - App secret.
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

	/** Disconnects from the current conference. Triggers OnDisconnected when complete. */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void Disconnect();

	/** Mutes audio input. Has no effect unless connected. */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void MuteInput();

	/** Unmutes audio input. Has no effect unless connected. */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void UnmuteInput();

	/** Mutes audio output. Has no effect unless connected. */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void MuteOutput();

	/** Unmutes audio output. Has no effect unless connected. */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void UnmuteOutput();

	/** Gets audio levels for all speaking participants. Triggers OnAudioLevelsChanged if successful. */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void GetAudioLevels();

	/** Updates the position and rotation of the listener for spatial audio purposes. Calling this function even once
	 * disables the default behavior, which is to automatically use the location and rotation of the first player
	 * controller.
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

	/** An initial or refreshed client access token is needed. Triggered when the game starts or when a refresh token is
	 * requested.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnTokenNeeded();

	/** The plugin is successfully initialized. Triggered by the SetToken function. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnInitialized();

	/** Successfully connected to conference. Triggered by the Connect function.
	 * @param LocalParticipant - The unique ID for the participant opening the session.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnConnected(const FString& LocalParticipantID);

	/** Disconnected from conference. Triggered when disconnected by any means (in particular by the Disconnect
	 * function).
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnDisconnected();

	/** Triggered when a remote participant is added to the conference.
	 * @param ParticipantInfo - Contains the current status of a conference participant and information whether the
	 * participant's audio is enabled.
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
	 * @param ActiveSpeakers - IDs of the current active speakers.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnActiveSpeakersChanged(const TArray<FString>& ActiveSpeakers);

	/** There are new audio levels available. Triggered by the GetAudioLevels function.
	 * @param AudioLevels - String-to-float mapping of participant IDs to their audio levels (0.0 is silent, 1.0 is
	 * loudest).
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
