// Copyright 2022 Dolby Laboratories

#pragma once

#include "Internationalization/Text.h"

namespace DolbyIO
{
	using FParticipant = FString;
	using FParticipants = TSet<FParticipant>;
	using FAudioLevels = TMap<FParticipant, float>;
}
