// Copyright 2022 Dolby Laboratories

#pragma once

#include "Internationalization/Text.h"

namespace Dolby
{
	using FToken = FString;
	using FConferenceName = FString;
	using FUserName = FString;
	using FMessage = FString;
	using FParticipant = FString;
	using FParticipants = TSet<FParticipant>;
	using FAudioLevels = TMap<FParticipant, float>;
	using FDeviceNames = TArray<FText>;
	using Index = int;
}
