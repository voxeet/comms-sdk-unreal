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
	class FVideoSink final : public dolbyio::comms::video_sink, public std::enable_shared_from_this<FVideoSink>
	{
	public:
		UTexture2D* GetTexture();

	private:
		void handle_frame(std::unique_ptr<dolbyio::comms::video_frame>) override;

		void RecreateIfNeeded(int Width, int Height);
		void Convert(dolbyio::comms::video_frame&);

		UTexture2D* Texture{};
		TArray<uint8> Buffer;
		FUpdateTextureRegion2D Region{0, 0, 0, 0, 0, 0};
		FRenderCommandFence Fence;
	};
}
