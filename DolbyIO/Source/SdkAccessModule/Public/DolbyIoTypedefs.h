// Copyright 2022 Dolby Laboratories

#pragma once

#include "Internationalization/Text.h"

namespace Dolby
{
	using FParticipant = FString;
	using FParticipants = TSet<FParticipant>;
	using FAudioLevels = TMap<FParticipant, float>;
	using FDeviceNames = TArray<FText>;
}
