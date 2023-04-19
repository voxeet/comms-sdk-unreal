// Copyright 2023 Dolby Laboratories

#include "DolbyIOVideoFrameHandler.h"

#include "Templates/UnrealTemplate.h"

namespace DolbyIO
{
	FVideoFrameHandler::FVideoFrameHandler(std::shared_ptr<dolbyio::comms::video_sink> Sink) : Sink(MoveTemp(Sink)) {}
}
