// Copyright 2023 Dolby Laboratories

#pragma once

#include "DolbyIOVideoTrack.generated.h"

/** Contains data about a Dolby.io video track. */
USTRUCT(BlueprintType, DisplayName = "Dolby.io Video Track")
struct DOLBYIO_API FDolbyIOVideoTrack
{
	GENERATED_BODY()

	/** The unique ID of the video track. */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	FString TrackID;

	/** The participant from whom the track is coming. */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	FString ParticipantID;

	/** Indicates whether this is a screenshare video track (true) or a camera video track (false). */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	bool bIsScreenshare{};
};
