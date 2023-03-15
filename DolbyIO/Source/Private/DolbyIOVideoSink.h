// Copyright 2023 Dolby Laboratories

#pragma once

#include "DolbyIOCppSdk.h"

#include <memory>

#include "RHI.h"
#include "RenderCommandFence.h"

class UMaterialInstanceDynamic;
class UTexture2D;

namespace DolbyIO
{
	class FVideoSink final : public dolbyio::comms::video_sink, public std::enable_shared_from_this<FVideoSink>
	{
	public:
		UTexture2D* GetTexture();
		void BindMaterial(UMaterialInstanceDynamic* Material);
		void UnbindMaterial(UMaterialInstanceDynamic* Material);

	private:
		void handle_frame(std::unique_ptr<dolbyio::comms::video_frame>) override;

		void RecreateIfNeeded(int Width, int Height);
		void Convert(dolbyio::comms::video_frame&);

		void UpdateMaterial(UMaterialInstanceDynamic* Material);

		UTexture2D* Texture{};
		TArray<uint8> Buffer;
		FUpdateTextureRegion2D Region{0, 0, 0, 0, 0, 0};
		FRenderCommandFence Fence;
		TSet<UMaterialInstanceDynamic*> Materials;
	};
}
