// Copyright 2023 Dolby Laboratories

#pragma once

#include "DolbyIOCppSdk.h"

#include "Containers/Set.h"

class UMaterialInstanceDynamic;
class UTexture2D;

namespace DolbyIO
{
	class FVideoSink final : public dolbyio::comms::video_sink, public std::enable_shared_from_this<FVideoSink>
	{
	public:
		UTexture2D* GetTexture() const;
		void BindMaterial(UMaterialInstanceDynamic* Material);
		void UnbindMaterial(UMaterialInstanceDynamic* Material);

	private:
		void handle_frame(std::unique_ptr<dolbyio::comms::video_frame>) override;

		void RecreateIfNeeded(int Width, int Height);
		void UpdateMaterial(UMaterialInstanceDynamic* Material);
		void Convert(dolbyio::comms::video_frame& VideoFrame);

		UTexture2D* Texture{};
		TSet<UMaterialInstanceDynamic*> Materials;
	};
}
