// Copyright 2022 Dolby Laboratories

#pragma once

#include "GameFramework/Actor.h"
#include "SdkEventsObserver.h"

#include "DolbyIoConference.generated.h"

namespace dolbyio
{
	namespace comms
	{
		class sdk;
	}
}

namespace Dolby
{
	class FSdkAccess;
}

/** Interface to the Dolby.io C++ SDK. On BeginPlay, initializes the Dolby.io C++ SDK using the client access token
 * property if set. */
UCLASS()
class DOLBYIOCONFERENCEMODULE_API ADolbyIoConference : public AActor, public Dolby::ISdkEventsObserver
{
	GENERATED_BODY()

public:
	ADolbyIoConference();

	// read-write values for controlling the SDK

	/** Dolby.io client access token. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dolby")
	FString Token;

	/** Alias of conference to join using the client access token. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dolby")
	FString ConferenceName;

	/** User name to be used in the conference. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dolby")
	FString UserName;

	/** Modifiable flag indicating whether to mute input audio. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dolby")
	bool bIsInputMuted;

	/** Modifiable flag indicating whether to mute output audio. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dolby")
	bool bIsOutputMuted;

	/** The position used to update the spatial audio configuration. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dolby")
	FVector Position;

	/** The rotation used to update the spatial audio configuration. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dolby")
	FRotator Rotation;

	// read-only values set by the SDK

	/** Description of the current status of the Dolby.io C++ SDK. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dolby")
	FString Status;

	/** List of all input audio devices registered in the Dolby.io C++ SDK. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dolby")
	TArray<FText> InputDevices;

	/** List of all output audio devices registered in the Dolby.io C++ SDK. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dolby")
	TArray<FText> OutputDevices;

	/** The index of the current input audio device used by the Dolby.io C++ SDK. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dolby")
	int CurrentInputDeviceIndex{0};

	/** The index of the current output audio device used by the Dolby.io C++ SDK. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dolby")
	int CurrentOutputDeviceIndex{0};

	/** The participant ID of the local user. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dolby")
	FString LocalParticipant;

	/** List of all remote participant IDs. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dolby")
	TSet<FString> RemoteParticipants;

	/** List of remote participants who are currently speaking. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dolby")
	TSet<FString> ActiveSpeakers;

	/** Map containing audio levels for participants who are speaking (key = participant ID, value = floating point
	 * number representing the audio level). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dolby")
	TMap<FString, float> AudioLevels;

	// functions for controlling the SDK, callable from Blueprints

	/** Connects to Dolby.io conference using the conference name and user name set using properties. */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void Connect();

	/** Disconnects from the current conference. */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void Disconnect();

	/** Mutes or unmutes audio input, depending on the value of the equivalent property. */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void MuteInput();

	/** Mutes or unmutes audio output, depending on the value of the equivalent property. */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void MuteOutput();

	/** Sets the input audio device.
	 * @param Index - Index of device from list of all input devices.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void SetInputDevice(const int Index);

	/** Sets the output audio device.
	 * @param Index - Index of device from list of all output devices.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void SetOutputDevice(const int Index);

	/** Requests the Dolby.io C++ SDK to get audio levels for all speaking participants. These will be available in the
	 * AudioLevels property once the OnNewAudioLevels event triggers. */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void GetAudioLevels();

	/** Refreshes the client access token set using the Token property. If the Dolby.io C++ SDK is not initialized,
	 * initializes using the Token instead. */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void RefreshToken();

	// events called from C++, implementable in Blueprints

	/** Event signaled when the status of the Dolby.io C++ SDK changes. */
	UFUNCTION(BlueprintNativeEvent, Category = "Dolby")
	void OnStatusChanged();

	/** Event signaled when there is a new list of input audio devices. Also called when a device is added or removed.
	 * The list always contains all available devices. */
	UFUNCTION(BlueprintNativeEvent, Category = "Dolby")
	void OnNewListOfInputDevices();

	/** Event signaled when there is a new list of output audio devices. Also called when a device is added or removed.
	 * The list always contains all available devices. */
	UFUNCTION(BlueprintNativeEvent, Category = "Dolby")
	void OnNewListOfOutputDevices();

	/** Event signaled when the input audio device is changed. */
	UFUNCTION(BlueprintNativeEvent, Category = "Dolby")
	void OnInputDeviceChanged();

	/** Event signaled when the output audio device is changed. */
	UFUNCTION(BlueprintNativeEvent, Category = "Dolby")
	void OnOutputDeviceChanged();

	/** Event signaled when the local participant ID is changed. */
	UFUNCTION(BlueprintNativeEvent, Category = "Dolby")
	void OnLocalParticipantChanged();

	/** Event signaled when there is a new list of remote participants available. */
	UFUNCTION(BlueprintNativeEvent, Category = "Dolby")
	void OnNewListOfRemoteParticipants();

	/** Event signaled when there is a new list of active speakers available. */
	UFUNCTION(BlueprintNativeEvent, Category = "Dolby")
	void OnNewListOfActiveSpeakers();

	/** Event signaled when there is a new map of audio levels available. */
	UFUNCTION(BlueprintNativeEvent, Category = "Dolby")
	void OnNewAudioLevels();

	/** Event signaled when the Dolby.io C++ SDK requests a refreshed token. */
	UFUNCTION(BlueprintNativeEvent, Category = "Dolby")
	void OnRefreshTokenNeeded();

	/** Event signaled when the position and rotation used to update the spatial audio configuration should be updated.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Dolby")
	void OnSpatialUpdateNeeded();

	// other functions

	/** Retrieves the pointer to the current Dolby.io C++ SDK instance to allow controlling the SDK at the lowest level.
	 */
	dolbyio::comms::sdk* GetRawSdk();

	// AActor
	void BeginPlay() override;
	void Tick(float DeltaTime) override;
	void EndPlay(const EEndPlayReason::Type) override;

protected:
	class APlayerController* FirstPlayerController;

private:
	void TriggerEvent(void (ADolbyIoConference::*Function)());

	// Dolby::ISdkEventsObserver
	void OnStatusChanged(const Dolby::FMessage& Status) override;
	void OnListOfInputDevicesChanged(const Dolby::FDeviceNames NewInputDevices) override;
	void OnListOfOutputDevicesChanged(const Dolby::FDeviceNames NewOutputDevices) override;
	void OnInputDeviceChanged(const int Index) override;
	void OnOutputDeviceChanged(const int Index) override;
	void OnLocalParticipantChanged(const Dolby::FParticipant& ParticipantId) override;
	void OnListOfRemoteParticipantsChanged(const Dolby::FParticipants& NewListOfParticipants) override;
	void OnListOfActiveSpeakersChanged(const Dolby::FParticipants Speakers) override;
	void OnAudioLevelsChanged(const Dolby::FAudioLevels NewAudioLevels) override;

	void OnRefreshTokenRequested() override;

	TSharedPtr<Dolby::FSdkAccess> CppSdk;
	FString PreviousToken;
};
