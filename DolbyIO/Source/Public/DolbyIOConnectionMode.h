// Copyright 2023 Dolby Laboratories

#pragma once

#include "DolbyIOConnectionMode.generated.h"

/** Defines whether to connect as an active user or a listener. */
UENUM(BlueprintType, DisplayName = "Dolby.io Connection Mode")
enum class EDolbyIOConnectionMode : uint8
{
	/** Join a conference as an active user. */
	Active,
	/** Receive multiple video streams from senders. */
	ListenerRegular,
	/** Receive a realtime mixed stream. */
	ListenerRTS,
};
