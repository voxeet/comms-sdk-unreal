// Copyright 2023 Dolby Laboratories

#pragma once

#include "Utils/DolbyIOCppSdk.h"

namespace DolbyIO
{
	class FVideoProcessingFrameHandler final : public dolbyio::comms::video_frame_handler,
	                                           public dolbyio::comms::video_source,
	                                           public dolbyio::comms::video_sink,
	                                           public std::enable_shared_from_this<FVideoProcessingFrameHandler>
	{
	public:
		FVideoProcessingFrameHandler(std::shared_ptr<dolbyio::comms::video_frame_handler> VideoFrameHandler,
		                             std::shared_ptr<dolbyio::comms::video_sink> PreviewSink)
		    : VideoFrameHandler(std::move(VideoFrameHandler)), PreviewSink(std::move(PreviewSink))
		{
		}

	private:
		std::shared_ptr<dolbyio::comms::video_sink> sink() override
		{
			return VideoFrameHandler->sink();
		}
		std::shared_ptr<dolbyio::comms::video_source> source() override
		{
			return shared_from_this();
		}

		void set_sink(const std::shared_ptr<video_sink>& Sink,
		              const dolbyio::comms::video_source::config& Config) override
		{
			SdkSink = Sink;
			VideoFrameHandler->source()->set_sink(shared_from_this(), Config);
		}

		void handle_frame(const dolbyio::comms::video_frame& VideoFrame) override
		{
			if (SdkSink)
			{
				SdkSink->handle_frame(VideoFrame);
				PreviewSink->handle_frame(VideoFrame);
			}
		}

		std::shared_ptr<dolbyio::comms::video_frame_handler> VideoFrameHandler;
		std::shared_ptr<dolbyio::comms::video_sink> PreviewSink;
		std::shared_ptr<dolbyio::comms::video_sink> SdkSink;
	};

};
