// Copyright 2023 Dolby Laboratories

#pragma once

#include "DolbyIOSpatialAudioStyle.generated.h"

/** The possible spatial audio styles of the conference. */
UENUM(BlueprintType, DisplayName = "Dolby.io Spatial Audio Style")
enum class EDolbyIOSpatialAudioStyle : uint8
{
	/** Spatial audio is disabled. */
	Disabled,
	/** The individual option sets the spatial location that is based on the spatial scene, local participant's
	 * position, and remote participants' positions. This allows a client to control the position using the local,
	 * self-contained logic. However, the client has to communicate a large set of requests constantly to the server,
	 * which increases network traffic, log subsystem pressure, and complexity of the client-side application.
	 */
	Individual,
	/** Spatial audio for shared scenes, The shared option sets the spatial location that is based on the spatial scene
	 * and the local participant's position, while the relative positions among participants are calculated by the
	 * Dolby.io server. This way, the spatial scene is shared by all participants, so that each client can set its own
	 * position and participate in the shared scene. This approach simplifies communication between the client and the
	 * server and decreases network traffic.
	 */
	Shared,
};
