// Copyright 2023 Dolby Laboratories

#include "DolbyIO.h"

#include "Video/DolbyIOVideoSink.h"

using namespace DolbyIO;

void UDolbyIOSubsystem::BindMaterial(UMaterialInstanceDynamic* Material, const FString& VideoTrackID)
{
	for (auto& Sink : VideoSinks)
	{
		if (Sink.Key != VideoTrackID)
		{
			Sink.Value->UnbindMaterial(Material);
		}
	}

	if (const std::shared_ptr<DolbyIO::FVideoSink>* Sink = VideoSinks.Find(VideoTrackID))
	{
		(*Sink)->BindMaterial(Material);
	}
}

void UDolbyIOSubsystem::UnbindMaterial(UMaterialInstanceDynamic* Material, const FString& VideoTrackID)
{
	if (const std::shared_ptr<DolbyIO::FVideoSink>* Sink = VideoSinks.Find(VideoTrackID))
	{
		(*Sink)->UnbindMaterial(Material);
	}
}

UTexture2D* UDolbyIOSubsystem::GetTexture(const FString& VideoTrackID)
{
	if (const std::shared_ptr<FVideoSink>* Sink = VideoSinks.Find(VideoTrackID))
	{
		return (*Sink)->GetTexture();
	}
	return nullptr;
}
