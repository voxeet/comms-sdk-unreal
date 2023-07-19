// Copyright 2023 Dolby Laboratories

#include "DolbyIO.h"

#include "Utils/DolbyIOBroadcastEvent.h"
#include "Utils/DolbyIOLogging.h"
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

void UDolbyIOSubsystem::BroadcastVideoTrackAdded(const FDolbyIOVideoTrack& VideoTrack)
{
	DLB_UE_LOG("Video track added: TrackID=%s ParticipantID=%s", *VideoTrack.TrackID, *VideoTrack.ParticipantID);
	BroadcastEvent(OnVideoTrackAdded, VideoTrack);
}

void UDolbyIOSubsystem::BroadcastVideoTrackEnabled(const FDolbyIOVideoTrack& VideoTrack)
{
	DLB_UE_LOG("Video track enabled: TrackID=%s ParticipantID=%s", *VideoTrack.TrackID, *VideoTrack.ParticipantID);
	BroadcastEvent(OnVideoTrackEnabled, VideoTrack);
}

void UDolbyIOSubsystem::ProcessBufferedVideoTracks(const FString& ParticipantID)
{
	if (TArray<FDolbyIOVideoTrack>* AddedTracks = BufferedAddedVideoTracks.Find(ParticipantID))
	{
		for (const FDolbyIOVideoTrack& AddedTrack : *AddedTracks)
		{
			VideoSinks[AddedTrack.TrackID]->OnTextureCreated(
			    [=]
			    {
				    BroadcastVideoTrackAdded(AddedTrack);

				    if (TArray<FDolbyIOVideoTrack>* EnabledTracks = BufferedEnabledVideoTracks.Find(ParticipantID))
				    {
					    TArray<FDolbyIOVideoTrack>& EnabledTracksRef = *EnabledTracks;
					    for (int i = 0; i < EnabledTracksRef.Num(); ++i)
					    {
						    if (EnabledTracksRef[i].TrackID == AddedTrack.TrackID)
						    {
							    BroadcastVideoTrackEnabled(EnabledTracksRef[i]);
							    EnabledTracksRef.RemoveAt(i);
							    if (!EnabledTracksRef.Num())
							    {
								    BufferedEnabledVideoTracks.Remove(ParticipantID);
							    }
							    return;
						    }
					    }
				    }
			    });
		}
		BufferedAddedVideoTracks.Remove(ParticipantID);
	}
}
