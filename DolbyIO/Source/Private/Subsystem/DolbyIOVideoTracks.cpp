// Copyright 2023 Dolby Laboratories

#include "DolbyIO.h"

#include "Utils/DolbyIOBroadcastEvent.h"
#include "Utils/DolbyIOConversions.h"
#include "Utils/DolbyIOErrorHandler.h"
#include "Utils/DolbyIOLogging.h"
#include "Video/DolbyIOVideoSink.h"

using namespace dolbyio::comms;
using namespace DolbyIO;

void UDolbyIOSubsystem::BindMaterial(UMaterialInstanceDynamic* Material, const FString& VideoTrackID)
{
	FScopeLock Lock{&VideoSinksLock};
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
	FScopeLock Lock{&VideoSinksLock};
	if (const std::shared_ptr<DolbyIO::FVideoSink>* Sink = VideoSinks.Find(VideoTrackID))
	{
		(*Sink)->UnbindMaterial(Material);
	}
}

UTexture2D* UDolbyIOSubsystem::GetTexture(const FString& VideoTrackID)
{
	FScopeLock Lock{&VideoSinksLock};
	if (const std::shared_ptr<FVideoSink>* Sink = VideoSinks.Find(VideoTrackID))
	{
		return (*Sink)->GetTexture();
	}
	return nullptr;
}

void UDolbyIOSubsystem::BroadcastVideoTrackAdded(const FDolbyIOVideoTrack& VideoTrack)
{
	DLB_UE_LOG("Video track added: TrackID=%s ParticipantID=%s IsScreenshare=%d", *VideoTrack.TrackID,
	           *VideoTrack.ParticipantID, VideoTrack.bIsScreenshare);
	WarnIfVideoTrackSuspicious(VideoTrack.TrackID);
	BroadcastEvent(VideoTrack.bIsScreenshare ? OnRemoteScreenshareTrackAdded : OnRemoteCameraTrackAdded,
	               VideoTrack.TrackID, VideoTrack.ParticipantID);
}

void UDolbyIOSubsystem::WarnIfVideoTrackSuspicious(const FString& VideoTrackID)
{
	if (VideoTrackID == "{-}")
	{
		DLB_UE_LOG_BASE(Warning, "Suspicious video track ID added, things may not work as expected");
	}
}

void UDolbyIOSubsystem::BroadcastRemoteCameraTrackEnabled(const FDolbyIOVideoTrack& VideoTrack)
{
	DLB_UE_LOG("Remote camera track enabled: TrackID=%s ParticipantID=%s", *VideoTrack.TrackID,
	           *VideoTrack.ParticipantID, VideoTrack.bIsScreenshare);
	BroadcastEvent(OnRemoteCameraTrackEnabled, VideoTrack.TrackID, VideoTrack.ParticipantID);
}

void UDolbyIOSubsystem::ProcessBufferedVideoTracks(const FString& ParticipantID)
{
	if (TArray<FDolbyIOVideoTrack>* AddedTracks = BufferedAddedVideoTracks.Find(ParticipantID))
	{
		FScopeLock Lock{&VideoSinksLock};
		for (const FDolbyIOVideoTrack& AddedTrack : *AddedTracks)
		{
			if (std::shared_ptr<DolbyIO::FVideoSink>* Sink = VideoSinks.Find(AddedTrack.TrackID))
			{
				(*Sink)->OnTextureCreated(
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
								    BroadcastRemoteCameraTrackEnabled(EnabledTracksRef[i]);
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
		}
		BufferedAddedVideoTracks.Remove(ParticipantID);
	}
}

void UDolbyIOSubsystem::Handle(const remote_video_track_added& Event)
{
	const FDolbyIOVideoTrack VideoTrack = ToFDolbyIOVideoTrack(Event.track);

	FScopeLock Lock1{&VideoSinksLock};
	VideoSinks.Emplace(VideoTrack.TrackID, std::make_shared<FVideoSink>(VideoTrack.TrackID));
	Sdk->video()
	    .remote()
	    .set_video_sink(Event.track, VideoSinks[VideoTrack.TrackID])
	    .on_error(DLB_ERROR_HANDLER_NO_DELEGATE);

	FScopeLock Lock2{&RemoteParticipantsLock};
	if (RemoteParticipants.Contains(VideoTrack.ParticipantID))
	{
		VideoSinks[VideoTrack.TrackID]->OnTextureCreated([this, VideoTrack] { BroadcastVideoTrackAdded(VideoTrack); });
	}
	else
	{
		DLB_UE_LOG("Buffering video track added: TrackID=%s ParticipantID=%s IsScreenshare=%d", *VideoTrack.TrackID,
		           *VideoTrack.ParticipantID, VideoTrack.bIsScreenshare);
		BufferedAddedVideoTracks.FindOrAdd(VideoTrack.ParticipantID).Add(VideoTrack);
	}
}

void UDolbyIOSubsystem::Handle(const remote_video_track_removed& Event)
{
	const FDolbyIOVideoTrack VideoTrack = ToFDolbyIOVideoTrack(Event.track);
	DLB_UE_LOG("Video track removed: TrackID=%s ParticipantID=%s IsScreenshare=%d", *VideoTrack.TrackID,
	           *VideoTrack.ParticipantID, Event.track.is_screenshare);

	FScopeLock Lock{&VideoSinksLock};
	if (std::shared_ptr<DolbyIO::FVideoSink>* Sink = VideoSinks.Find(VideoTrack.TrackID))
	{
		(*Sink)->UnbindAllMaterials();
		VideoSinks.Remove(VideoTrack.TrackID);
	}
	else
	{
		DLB_UE_LOG_BASE(Warning, "Non-existent video track removed");
	}

	BroadcastEvent(Event.track.is_screenshare ? OnRemoteScreenshareTrackRemoved : OnRemoteCameraTrackRemoved,
	               VideoTrack.TrackID, VideoTrack.ParticipantID);
}

void UDolbyIOSubsystem::Handle(const utils::vfs_event& Event)
{
	for (const auto& TrackMapItem : Event.new_enabled)
	{
		const FDolbyIOVideoTrack VideoTrack = ToFDolbyIOVideoTrack(TrackMapItem);

		if (GetTexture(VideoTrack.TrackID))
		{
			BroadcastRemoteCameraTrackEnabled(VideoTrack);
		}
		else
		{
			DLB_UE_LOG("Buffering video track enabled: TrackID=%s ParticipantID=%s IsScreenshare=%d",
			           *VideoTrack.TrackID, *VideoTrack.ParticipantID, VideoTrack.bIsScreenshare);
			BufferedEnabledVideoTracks.FindOrAdd(VideoTrack.ParticipantID).Add(VideoTrack);
		}
	}
	for (const auto& TrackMapItem : Event.new_disabled)
	{
		const FDolbyIOVideoTrack VideoTrack = ToFDolbyIOVideoTrack(TrackMapItem);
		DLB_UE_LOG("Video track ID %s for participant ID %s disabled", *VideoTrack.TrackID, *VideoTrack.ParticipantID);
		BroadcastEvent(OnRemoteCameraTrackDisabled, VideoTrack.TrackID, VideoTrack.ParticipantID);
	}
}
