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
		FVideoSink(const FString& VideoTrackID);
		~FVideoSink();

		UTexture2D* GetTexture();
		void BindMaterial(UMaterialInstanceDynamic* Material);
		void UnbindMaterial(UMaterialInstanceDynamic* Material);
		void UnbindAllMaterials();

	private:
		void handle_frame(const dolbyio::comms::video_frame&) override;

		void Convert(const dolbyio::comms::video_frame& VideoFrame);
		void Render(const dolbyio::comms::video_frame& VideoFrame);

		UTexture2D* const Texture{};
		TArray<uint8> Buffer;

		TSet<UMaterialInstanceDynamic*> Materials;
		const FString VideoTrackID;
	};
}
