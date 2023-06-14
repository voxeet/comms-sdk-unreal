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
	int64 ID{};

	/** Indicates whether this is an entire screen (true) or an application window (false). */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	bool bIsScreen{};

	/** Title of the screen. */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	FText Title;
};

/** The encoder hint provides a hint to the plugin as to what type of content is being captured by the screen share.
 * This can be used to help when making decisions for choosing encoder settings based on what aspects of the content are
 * important. For instance, if network conditions worsen is it more important to keep a higher resolution of screen
 * share or frame rate to keep the motion more fluid.
 */
UENUM(BlueprintType, DisplayName = "Dolby.io Screenshare Encoder Hint")
enum class EDolbyIOScreenshareEncoderHint : uint8
{
	/** The content hint is not specified, encoder will choose settings based on the fact track is screen share only. */
	Unspecified,
	/** Content is detailed, encoder should keep resolution high if it can. */
	Detailed,
	/** Content is text, encoder should keep resolution high if it can. */
	Text,
	/** Content is fluid, full of motion, encoder should keep frame rate higher. */
	Fluid,
};

/** The max resolution for the capture screen content to be shared as. If the actual captured resolution is higher than
 * the set max resolution the plugin will downscale to the max resolution. The plugin captures screen content based on
 * the chosen display's resolution. The max resolution will also be downscaled if the captured resolution is higher,
 * otherwise this has no effect. Therefore the screen content will never be shared at a resolution higher than the one
 * set here, but if the captured display has lower resolution the content will not be upscaled.
 */
UENUM(BlueprintType, DisplayName = "Dolby.io Screenshare Max Resolution")
enum class EDolbyIOScreenshareMaxResolution : uint8
{
	/** Downscale the height to 1080p, the width will be chosen to respect the actual aspect ratio of the raw captured
	 * frame. Downscaling will only happen if the captured resolution is higher. */
	DownscaleTo1080p,
	/** Downscale the height to 1440p, the width will be chosen to respect the actual aspect ratio of the raw captured
	 * frame. Downscaling will only happen if the captured resolution is higher. */
	DownscaleTo1440p,
	/** No downscaling, send the content to the encoder in the actual captured resolution. */
	ActualCaptured,
};

/** The quality for the downscaling algorithm to be used. The higher the quality, the clearer the picture will be, but
 * the higher the CPU usage will be.
 */
UENUM(BlueprintType, DisplayName = "Dolby.io Screenshare Downscale Quality")
enum class EDolbyIOScreenshareDownscaleQuality : uint8
{
	/** Use the quicker algorithm, fast and lowest CPU used but low quality. */
	Low,
	/** Use the linear filter algorithm used mainly for horizontal scaling. */
	Medium,
	/** Use the bilinear filter algorithm, faster than highest, but quality not as good. If downscaling fluid screen
	 * content from 4K to 1080p this option should be used.
	 */
	High,
	/** Use the box filter algorithm, uses the most CPU and is the slowest but produces the best quality. If downscaling
	 * detailed screen content from 4K to 2K this option should be used.
	 */
	Highest,
};
