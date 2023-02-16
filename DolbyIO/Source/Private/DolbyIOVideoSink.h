// Copyright 2023 Dolby Laboratories

#pragma once

#include "HAL/Platform.h"

#if PLATFORM_WINDOWS
#pragma warning(push)
// 'type' : class 'type1' needs to have dll-interface to be used by clients of class 'type2'
#pragma warning(disable : 4251)
// non - DLL-interface class 'class_1' used as base for DLL-interface class 'class_2'
#pragma warning(disable : 4275)
#elif PLATFORM_MAC
#define DOLBYIO_COMMS_SUPPRESS_APPLE_NO_RTTI_WARNING
#endif

#include <dolbyio/comms/sdk.h>

#if PLATFORM_WINDOWS
#pragma warning(pop)
#endif

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
			void Update();

			UTexture2D* Texture{};
			TArray<uint8> Buffer;
			FUpdateTextureRegion2D Region{0, 0, 0, 0, 0, 0};
		};

		void handle_frame(const std::string&, const std::string&,
		                  std::unique_ptr<dolbyio::comms::video_frame>) override;

		FCriticalSection FrameLock;
		TMap<FString, FFrame> StreamFrames;
		TMap<FString, FString> ParticipantStreams;
	};
}
