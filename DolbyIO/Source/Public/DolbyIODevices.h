// Copyright 2023 Dolby Laboratories

#pragma once

#include "DolbyIODevices.generated.h"

/** The platform-agnostic description of an audio device. */
USTRUCT(BlueprintType, DisplayName = "Dolby.io Audio Device")
struct DOLBYIO_API FDolbyIOAudioDevice
{
	GENERATED_BODY()

	/** The name of the device. */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	FText Name;

	/** The native ID of the device. */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	FString NativeID;
};

/** The platform agnostic description of a video device. */
USTRUCT(BlueprintType, DisplayName = "Dolby.io Video Device")
struct DOLBYIO_API FDolbyIOVideoDevice
{
	GENERATED_BODY()

	/** The name of the device. */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	FText Name;

	/** The unique ID of the device. */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	FString UniqueID;
};
