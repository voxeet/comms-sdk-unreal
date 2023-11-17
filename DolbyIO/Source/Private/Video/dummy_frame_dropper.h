// Copyright 2023 Dolby Laboratories

#pragma once

#include "Utils/DolbyIOCppSdk.h"

#include <mutex>

namespace DolbyIO
{
	class dummy_frame_dropper final : public dolbyio::comms::video_source,
					  public dolbyio::comms::video_sink,
					  public dolbyio::comms::video_frame_handler,
					  public std::enable_shared_from_this<dummy_frame_dropper>
	{
	public:
		static constexpr int frame_rate_ = 30;
		dummy_frame_dropper(std::shared_ptr<dolbyio::comms::video_sink> sink, int fps)
			: local_sink_(std::move(sink)), desired_frame_rate_(fps)
		{
			assert(desired_frame_rate_);
			assert(local_sink_);
			frame_to_pass_back_ = frame_rate_ / desired_frame_rate_;
		}
		~dummy_frame_dropper() = default;

		void handle_frame(const dolbyio::comms::video_frame& frame) override
		{
			std::lock_guard<std::mutex> lock(sink_lock_);
			if (sdk_video_sink_)
			{
				// Only provide frame back to SDK if it the desired fps frame
				if (++frame_counter_ >= frame_to_pass_back_)
				{
					sdk_video_sink_->handle_frame(frame);
					frame_counter_ = 0;
				}
			}
			local_sink_->handle_frame(frame);
		}
		void set_sink(const std::shared_ptr<video_sink>& sink, const config&) override
		{
			std::lock_guard<std::mutex> lock(sink_lock_);
			sdk_video_sink_ = sink;
		}
		std::shared_ptr<video_sink> sink() override
		{
			return shared_from_this();
		}
		std::shared_ptr<video_source> source() override
		{
			return shared_from_this();
		}

	private:
		std::mutex sink_lock_;
		std::shared_ptr<dolbyio::comms::video_sink> local_sink_{};
		std::shared_ptr<dolbyio::comms::video_sink> sdk_video_sink_{};
		int desired_frame_rate_;
		int frame_to_pass_back_;
		int frame_counter_{0};
	};
}
