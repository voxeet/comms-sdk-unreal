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
									 std::shared_ptr<dolbyio::comms::video_sink> PreviewSink, int fps)
			: desired_fps_(fps), VideoFrameHandler(std::move(VideoFrameHandler)), PreviewSink(std::move(PreviewSink))
		{
			assert(desired_fps_);
			pass_every_n_ = 30 / desired_fps_;
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
			{
				std::lock_guard<std::mutex> lock(sdk_lock_);
				SdkSink = Sink;
			}
			VideoFrameHandler->source()->set_sink(shared_from_this(), Config);
		}

		void handle_frame(const dolbyio::comms::video_frame& VideoFrame) override
		{
			std::lock_guard<std::mutex> lock(sdk_lock_);
			if (SdkSink)
			{
				if (++frames_counter_ >= pass_every_n_)
				{
					SdkSink->handle_frame(VideoFrame);
					frames_counter_ = 0;
				}
			}
			PreviewSink->handle_frame(VideoFrame);
		}

		std::mutex sdk_lock_;
		int desired_fps_{0};
		int frames_counter_{0};
		int pass_every_n_{0};
		std::shared_ptr<dolbyio::comms::video_frame_handler> VideoFrameHandler;
		std::shared_ptr<dolbyio::comms::video_sink> PreviewSink;
		std::shared_ptr<dolbyio::comms::video_sink> SdkSink;
	};

};
