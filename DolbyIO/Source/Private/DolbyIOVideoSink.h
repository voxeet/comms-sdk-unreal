// Copyright 2023 Dolby Laboratories

#pragma once

#include <dolbyio/comms/media_engine/media_engine.h>

#include "RHI.h"

class UTexture2D;

namespace DolbyIO
{
	class FVideoSink final : public dolbyio::comms::video_sink
	{
	public:
		void AddStream(const FString& ParticipantID, const FString& StreamID);
		void RemoveStream(const FString& ParticipantID, const FString& StreamID);

		UTexture2D* GetTexture(const FString& ParticipantID);

	private:
		struct FFrame
		{
			void RecreateIfNeeded(int Width, int Height);
			void Convert(dolbyio::comms::video_frame&);

			UTexture2D* Texture{};
			TArray<uint8> Buffer;
			FUpdateTextureRegion2D Region{0, 0, 0, 0, 0, 0};
		};

		void handle_frame(const std::string&, const std::string&,
		                  std::unique_ptr<dolbyio::comms::video_frame>) override;

		TMap<FString, FFrame> Frames;
		TMap<FString, FString> ParticipantStreams;
	};
}
