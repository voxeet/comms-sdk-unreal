// Copyright 2023 Dolby Laboratories

#pragma once

#include "DolbyIOCppSdk.h"

namespace DolbyIO
{
	class FVideoFrameHandler final : public dolbyio::comms::video_frame_handler
	{
	public:
		FVideoFrameHandler(std::shared_ptr<dolbyio::comms::video_sink> Sink) : Sink(std::move(Sink)) {}

	private:
		std::shared_ptr<dolbyio::comms::video_sink> sink() override
		{
			return Sink;
		}
		std::shared_ptr<dolbyio::comms::video_source> source() override
		{
			return nullptr;
		}

		std::shared_ptr<dolbyio::comms::video_sink> Sink;
	};
}
