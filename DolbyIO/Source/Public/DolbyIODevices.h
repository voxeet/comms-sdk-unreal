// Copyright 2023 Dolby Laboratories

#pragma once

#include "DolbyIODevices.generated.h"

/** The platform agnostic description of audio device. */
USTRUCT(BlueprintType, DisplayName = "Dolby.io Audio Device")
struct DOLBYIO_API FDolbyIOAudioDevice
{
	GENERATED_BODY()

	/** Name of the device. */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	FText Name;

	/** Native ID of the device. */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	FString NativeId;

	/** Sdk ID of the device. */
	TSharedPtr<struct FSdkId> SdkId;
};

/** The platform agnostic description of video device. */
USTRUCT(BlueprintType, DisplayName = "Dolby.io Video Device")
struct DOLBYIO_API FDolbyIOVideoDevice
{
	GENERATED_BODY()

	/** Name of the device. */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	FText Name;

	/** Unique ID of the device. */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	FString UniqueId;
};
