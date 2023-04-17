// Copyright 2023 Dolby Laboratories

#include "DolbyIOVideoFrameHandler.h"

#include "Templates/UnrealTemplate.h"

namespace DolbyIO
{
	FVideoFrameHandler::FVideoFrameHandler() : Sink(std::make_shared<FVideoSink>()) {}
}
