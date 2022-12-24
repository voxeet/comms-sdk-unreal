// Copyright 2022 Dolby Laboratories

#pragma once

#include "DolbyIO/SdkEventObserver.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "Engine/EngineTypes.h"

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

	/** Sets the client access token. Initializes the plugin unless already initialized. Triggers OnInitialized if
	 * initialization was successful.
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

	/** Disconnects from the current conference. Triggers OnDisconnected if successful. */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void Disconnect();

	/** Mutes audio input.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void MuteInput();

	/** Unmutes audio input.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void UnmuteInput();

	/** Mutes audio output.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void MuteOutput();

	/** Unmutes audio output.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void UnmuteOutput();

	/** Gets audio levels for all speaking participants. Triggers OnListOfAudioLevelsChanged if successful.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void GetAudioLevels();

	/** Updates the position and rotation for spatial audio purposes. Calling this function even once disables the
	 * default behavior, which is to automatically use the location and rotaion of the first player controller.
	 * @param Position - The location of the listener.
	 * @param Rotation - The rotation of the listener.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void UpdateViewPoint(const FVector& Position, const FRotator& Rotation);

	/** Sets the input audio device. Triggers OnCurrentInputDeviceChanged if successful.
	 * @param Index - Index of device from list of all input devices.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void SetInputDevice(int Index);

	/** Sets the output audio device. Triggers OnCurrentOutputDeviceChanged if successful.
	 * @param Index - Index of device from list of all output devices.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void SetOutputDevice(int Index);

	/*
	 *
	 * Events triggered from C++, implementable in Blueprints.
	 *
	 */

	/** An initial or refreshed client access token is needed. Triggered at game start or when a
	 * refresh token is requested.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnTokenNeeded();

	/** The plugin is successfully initialized. Triggered by the SetToken function.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnInitialized();

	/** Successfully connected to conference. Triggered by the Connect function.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnConnected();

	/** Disconnected from conference. Triggered by the Disconnect function or when errors occur.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnDisconnected();

	/** The local participant ID has changed. Triggered by the Connect function, but does not mean that connection was
	 * successful.
	 * @param LocalParticipant - String holding the ID of the local participant (you).
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnLocalParticipantChanged(const FString& LocalParticipant);

	/** There is a new list of remote participants available. Triggered when remote participants are added to or removed
	 * from the conference.
	 * @param LocalParticipant - Set of strings holding the IDs of the remote participants (them).
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnListOfRemoteParticipantsChanged(const TSet<FString>& RemoteParticipants);

	/** There is a new list of active speakers available. Triggered when participants start or stop speaking.
	 * @param ActiveSpeakers - Set of strings holding the IDs of the current active speakers.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnListOfActiveSpeakersChanged(const TSet<FString>& ActiveSpeakers);

	/** There are new audio levels available. Triggered by the GetAudioLevels function.
	 * @param AudioLevels - String-to-float mapping of participant IDs to their audio levels (0.0 is silent, 1.0 is
	 * loudest).
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnListOfAudioLevelsChanged(const TMap<FString, float>& AudioLevels);

	/** There is a new list of input audio devices. Triggered when the initial list of devices is available (around the
	 * same time as OnInitialized) or when a device is added or removed. The list always contains all available devices.
	 * @param InputDevices - Array of input device names.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnListOfInputDevicesChanged(const TArray<FText>& InputDevices);

	/** There is a new list of output audio devices. Triggered when the initial list of devices is available (around the
	 * same time as OnInitialized) or when a device is added or removed. The list always contains all available devices.
	 * @param OutputDevices - Array of output device names.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnListOfOutputDevicesChanged(const TArray<FText>& OutputDevices);

	/** The current input audio device has changed. Triggered by the SetInputDevice function or when the current device
	 * is removed.
	 * @param Index - Index of device from list of all input devices.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnCurrentInputDeviceChanged(int Index);

	/** The current output audio device has changed. Triggered by the SetOutputDevice function or when the current
	 * device is removed.
	 * @param Index - Index of device from list of all output devices.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnCurrentOutputDeviceChanged(int Index);

private:
	void UpdateViewPointUsingFirstPlayer();

	// UGameInstanceSubsystem
	void Initialize(FSubsystemCollectionBase&) override;
	void Deinitialize() override;

	// ISdkEventObserver
	void OnTokenNeededEvent() override;
	void OnInitializedEvent() override;
	void OnConnectedEvent() override;
	void OnDisconnectedEvent() override;

	void OnLocalParticipantChangedEvent(const DolbyIO::FParticipant&) override;
	void OnListOfRemoteParticipantsChangedEvent(const DolbyIO::FParticipants&) override;
	void OnListOfActiveSpeakersChangedEvent(const DolbyIO::FParticipants&) override;
	void OnListOfAudioLevelsChangedEvent(const DolbyIO::FAudioLevels&) override;

	void OnListOfInputDevicesChangedEvent(const DolbyIO::FDeviceNames&) override;
	void OnListOfOutputDevicesChangedEvent(const DolbyIO::FDeviceNames&) override;
	void OnCurrentInputDeviceChangedEvent(int Index) override;
	void OnCurrentOutputDeviceChangedEvent(int Index) override;

	TSharedPtr<DolbyIO::FSdkAccess> CppSdk;
	TSharedPtr<DolbyIO::FAuthenticator> Authenticator;

	class UGameInstance* GameInstance;
	FTimerHandle SpatialUpdateTimerHandle;
};
