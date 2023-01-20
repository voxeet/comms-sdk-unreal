// Copyright 2023 Dolby Laboratories

#include "DolbyIOVideoSink.h"

#include "DolbyIOLogging.h"

#include <dolbyio/comms/media_engine/video_utils.h>

#include "Engine/Texture2D.h"

namespace DolbyIO
{
	void FVideoSink::AddStream(const FString& ParticipantID, const FString& StreamID)
	{
		DLB_UE_LOG(Log, "Video track added: ParticipantId=%s StreamId=%s", *ParticipantID, *StreamID);
		Frames.Emplace(StreamID, FFrame{ParticipantID});
		ParticipantStreams.Emplace(ParticipantID, StreamID);
	}

	void FVideoSink::RemoveStream(const FString& ParticipantID, const FString& StreamID)
	{
		DLB_UE_LOG(Log, "Video track removed: ParticipantId=%s StreamId=%s", *ParticipantID, *StreamID);
		if (FFrame* Frame = Frames.Find(StreamID))
		{
			Frame->Texture->ReleaseResource();
		}
		Frames.Remove(StreamID);
		ParticipantStreams.Remove(ParticipantID);
	}

	UTexture2D* FVideoSink::GetTexture(const FString& ParticipantID)
	{
		if (FString* StreamID = ParticipantStreams.Find(ParticipantID))
		{
			if (FFrame* Frame = Frames.Find(*StreamID))
			{
				return Frame->Texture;
			}
		}
		return nullptr;
	}

	FVideoSink::FFrame::FFrame(const FString& ParticipantID)
	    : ParticipantID(ParticipantID), Region(0, 0, 0, 0, 0, 0), Texture(nullptr)
	{
	}

	constexpr int Stride = 4;

	void FVideoSink::FFrame::RecreateIfNeeded(int Width, int Height)
	{
		FTexturePlatformData** FrameData = Texture ? Texture->GetRunningPlatformData() : nullptr;
		if (FrameData && *FrameData && (**FrameData).SizeX == Width && (**FrameData).SizeY == Height)
		{
			return;
		}

		Region.Width = Width;
		Region.Height = Height;
		Buffer.SetNumUninitialized(Width * Height * Stride);
		Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
		Texture->Filter = TextureFilter::TF_Trilinear;
#if WITH_EDITOR
		Texture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
#endif
		Texture->SRGB = 1;
		Texture->UpdateResource();
	}

	void FVideoSink::FFrame::Convert(dolbyio::comms::video_frame& VideoFrame)
	{
		if (dolbyio::comms::video_frame_i420* FrameI420 = VideoFrame.get_i420_frame())
		{
			dolbyio::comms::video_utils::format_converter::i420_to_argb(
			    FrameI420->get_y(), FrameI420->stride_y(), FrameI420->get_u(), FrameI420->stride_u(),
			    FrameI420->get_v(), FrameI420->stride_v(), Buffer.GetData(), VideoFrame.width() * Stride,
			    VideoFrame.width(), VideoFrame.height());
		}
	}

	void FVideoSink::handle_frame(const std::string& StreamID, const std::string& TrackID,
	                              std::unique_ptr<dolbyio::comms::video_frame> VideoFrame)
	{
		if (FFrame* Frame = Frames.Find(StreamID.c_str()))
		{
			Frame->RecreateIfNeeded(VideoFrame->width(), VideoFrame->height());
			Frame->Convert(*VideoFrame);
			Frame->Texture->UpdateTextureRegions(0, 1, &Frame->Region, VideoFrame->width() * Stride, Stride,
			                                     Frame->Buffer.GetData());
		}
	}
}
