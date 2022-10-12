#pragma once

#include "SdkStatusObserver.h"

#include "GameFramework/Actor.h"

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

/** Interface to the Dolby.io C++ SDK. */
UCLASS()
class DOLBYIOCONFERENCEMODULE_API ADolbyIoConference : public AActor, public Dolby::ISdkStatusObserver
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

	/** The current input audio device used by the Dolby.io C++ SDK. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dolby")
	FText CurrentInputDevice;

	/** The current output audio device used by the Dolby.io C++ SDK. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dolby")
	FText CurrentOutputDevice;

	// functions for controlling the SDK, callable from Blueprints

	/** Connects to Dolby.io conference using the client access token, conference name and user name set using
	 * properties. */
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

	/** Refreshes the client access token set using the Token property. */
	UFUNCTION(BlueprintCallable, Category = "Dolby")
	void RefreshToken();

	// events called from C++, implementable in Blueprints

	/** Event signaled when the status of the Dolby.io C++ SDK changes. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnStatusChanged();

	/** Event signaled when there is a new list of input audio devices. Also called when a device is added or removed.
	 * The list always contains all available devices. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnNewListOfInputDevices();

	/** Event signaled when there is a new list of output audio devices. Also called when a device is added or removed.
	 * The list always contains all available devices. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnNewListOfOutputDevices();

	/** Event signaled when the input audio device is changed. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnInputDeviceChanged();

	/** Event signaled when the output audio device is changed. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnOutputDeviceChanged();

	/** Event signaled when the Dolby.io C++ SDK requests a refreshed token. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Dolby")
	void OnRefreshTokenNeeded();

	// events called from C++ with a default C++ implementation, overridable in Blueprints

	/** Event signaled when the position and rotation used to update the spatial audio configuration should be updated.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Dolby")
	void OnSpatialUpdateNeeded();

	// other functions

	/** Retrieves the pointer to the current Dolby.io C++ SDK instance to allow controlling the SDK at the lowest level.
	 */
	dolbyio::comms::sdk* GetRawSdk();

private:
	// AActor
	void Tick(float DeltaTime) override;

	// Dolby::ISdkStatusObserver
	void OnStatusChanged(const FMessage&) override;

	void OnNewListOfInputDevices(const FDeviceNames&) override;
	void OnNewListOfOutputDevices(const FDeviceNames&) override;
	void OnInputDeviceChanged(const FDeviceName&) override;
	void OnOutputDeviceChanged(const FDeviceName&) override;

	void OnRefreshTokenRequested() override;

	TSharedPtr<Dolby::FSdkAccess> CppSdk;
};
