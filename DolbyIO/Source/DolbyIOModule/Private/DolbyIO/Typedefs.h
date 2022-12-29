// Copyright 2022 Dolby Laboratories

#pragma once

#include "Containers/Map.h"
#include "Containers/UnrealString.h"

namespace DolbyIO
{
	using FParticipantID = FString;
	using FParticipantIDs = TArray<FParticipantID>;
	using FAudioLevels = TMap<FParticipantID, float>;
}
