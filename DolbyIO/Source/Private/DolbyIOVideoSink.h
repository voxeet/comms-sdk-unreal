// Copyright 2023 Dolby Laboratories

#pragma once

#include "DolbyIOCppSdk.h"

#include "Containers/Set.h"
#include "Templates/SharedPointer.h"

class UMaterialInstanceDynamic;
class UTexture2D;

namespace DolbyIO
{
	class FVideoSink final : public dolbyio::comms::video_sink
	{
	public:
		FVideoSink(const FString& VideoTrackID);

		UTexture2D* GetTexture();
		void BindMaterial(UMaterialInstanceDynamic* Material);
		void UnbindMaterial(UMaterialInstanceDynamic* Material);
		void UnbindAllMaterials();

	private:
		void handle_frame(const dolbyio::comms::video_frame&) override;

		void CreateTexture(int Width, int Height);
		void ResizeTexture(int Width, int Height);
		void Convert(const dolbyio::comms::video_frame& VideoFrame);

		TSharedPtr<class FVideoTexture> Texture;
		TSet<UMaterialInstanceDynamic*> Materials;
		const FString VideoTrackID;
	};
}
