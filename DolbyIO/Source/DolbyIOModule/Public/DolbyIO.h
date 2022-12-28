// Copyright 2022 Dolby Laboratories

#pragma once

#include "DolbyIO/SdkEventObserver.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "Engine/EngineTypes.h"
#include "HAL/ThreadSafeBool.h"

#include "DolbyIO.generated.h"

namespace DolbyIO
{
	class FAuthenticator;
	class FSdkAccess;
}

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
	 * @param UserName - User name.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void Connect(const FString& ConferenceName = "unreal", const FString& UserName = "unreal");

	/** Disconnects from the current conference. Triggers OnDisconnected when complete. */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void Disconnect();

	/** Mutes audio input. Has no effect unless connected.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void MuteInput();

	/** Unmutes audio input. Has no effect unless connected.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void UnmuteInput();

	/** Mutes audio output. Has no effect unless connected.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void MuteOutput();

	/** Unmutes audio output. Has no effect unless connected.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void UnmuteOutput();

	/** Gets audio levels for all speaking participants. Triggers OnAudioLevelsChanged if successful.
	 */
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

	/** The plugin is successfully initialized. Triggered by the SetToken function.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnInitialized();

	/** Successfully connected to conference. Triggered by the Connect function.
	 * @param LocalParticipant - String holding the ID of the local participant (you).
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnConnected(const FString& LocalParticipant);

	/** Disconnected from conference. Triggered when disconnected by any means (in particular by the Disconnect
	 * function).
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnDisconnected();

	/** Triggered when participants are added to or removed from the conference.
	 * @param RemoteParticipants - Set of strings holding the IDs of the remote participants (them).
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnRemoteParticipantsChanged(const TSet<FString>& RemoteParticipants);

	/** Triggered when participants start or stop speaking.
	 * @param ActiveSpeakers - Set of strings holding the IDs of the current active speakers.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnActiveSpeakersChanged(const TSet<FString>& ActiveSpeakers);

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
	void OnConnectedEvent(const DolbyIO::FParticipant&) override;
	void OnDisconnectedEvent() override;
	void OnRemoteParticipantsChangedEvent(const DolbyIO::FParticipants&) override;
	void OnActiveSpeakersChangedEvent(const DolbyIO::FParticipants&) override;
	void OnAudioLevelsChangedEvent(const DolbyIO::FAudioLevels&) override;

	TSharedPtr<DolbyIO::FSdkAccess> CppSdk;
	TSharedPtr<DolbyIO::FAuthenticator> Authenticator;

	class UGameInstance* GameInstance;
	FTimerHandle SpatialUpdateTimerHandle;
	FThreadSafeBool bIsAlive = true;
};
