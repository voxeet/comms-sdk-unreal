// Copyright 2023 Dolby Laboratories

#pragma once

#include "DolbyIOVideoForwardingStrategy.generated.h"

/** Defines how the plugin should select conference participants whose videos will be transmitted to the local
 * participant.
 */
UENUM(BlueprintType, DisplayName = "Dolby.io Video Forwarding Strategy")
enum class EDolbyIOVideoForwardingStrategy : uint8
{
	/** Selects participants based on their audio volume.
	 */
	LastSpeaker,
	/** Selects participants based on the distance from the local participant. This strategy is available only for
	 * participants who enabled spatial audio.
	 */
	ClosestUser
};
