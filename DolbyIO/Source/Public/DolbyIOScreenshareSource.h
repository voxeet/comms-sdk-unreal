// Copyright 2023 Dolby Laboratories

#pragma once

#include "DolbyIOScreenshareSource.generated.h"

/** The platform agnostic description of source for screen sharing. */
USTRUCT(BlueprintType, DisplayName = "Dolby.io Screenshare Source")
struct DOLBYIO_API FDolbyIOScreenshareSource
{
	GENERATED_BODY()

	/** Unique id of the screen in question. */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	int64 ID;

	/** Indicates whether this is an entire screen (true) or an application window (false). */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	bool bIsScreen;

	/** Title of the screen. */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	FText Title;
};

/** The screen share content type provides a hint to the SDK as to what type of content is being captured by the screen
 * share. This can be used to help when making decisions for choosing encoder settings based on what aspects of the
 * content are important. For instance if network conditions worsen is it more important to keep a higher resolution of
 * screen share or frame rate to keep the motion more fluid. */
UENUM(BlueprintType, DisplayName = "Dolby.io Screenshare Content Type")
enum class EDolbyIOScreenshareContentType : uint8
{
	/** The default value not specifying content type. */
	Unspecified,
	/** Content is very detailed, encoder should keep resolution high if it can. */
	Detailed,
	/** Content is text. */
	Text,
	/** Content is fluid like a video for instance, encoder should keep frame rate higher. */
	Fluid,
};
