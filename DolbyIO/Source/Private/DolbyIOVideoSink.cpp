// Copyright 2023 Dolby Laboratories

#include "DolbyIOVideoSink.h"

#include "DolbyIOLogging.h"

#if PLATFORM_MAC
#include <dolbyio/comms/media_engine/video_frame_macos.h>
#endif
#include <dolbyio/comms/media_engine/video_utils.h>

#include "Async/Async.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace DolbyIO
{
	UTexture2D* FVideoSink::GetTexture()
	{
		return Texture;
	}

	void FVideoSink::BindMaterial(UMaterialInstanceDynamic* Material)
	{
		Materials.Add(Material);
		UpdateMaterial(Material);
	}

	void FVideoSink::UnbindMaterial(UMaterialInstanceDynamic* Material)
	{
		if (Materials.Remove(Material) && IsValid(Material))
		{
			Material->SetTextureParameterValue("DolbyIO Frame", UTexture2D::CreateTransient(1, 1, PF_B8G8R8A8));
		}
	}

	constexpr int Stride = 4;

	void FVideoSink::RecreateIfNeeded(int Width, int Height)
	{
		if (Texture)
		{
			if (Texture->GetSizeX() == Width && Texture->GetSizeY() == Height)
			{
				return;
			}
			else
			{
				DLB_UE_LOG("Recreating texture: old %dx%d new %dx%d", Texture->GetSizeX(), Texture->GetSizeY(), Width,
				           Height);
				Texture->RemoveFromRoot();
			}
		}
		else
		{
			DLB_UE_LOG("Creating texture: %dx%d", Width, Height);
			Fence.BeginFence();
		}

		Region.Width = Width;
		Region.Height = Height;
		Buffer.SetNumUninitialized(Width * Height * Stride);
		Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
		Texture->AddToRoot();
		Texture->UpdateResource();

		for (UMaterialInstanceDynamic* Material : Materials)
		{
			UpdateMaterial(Material);
		}
	}

	void FVideoSink::Convert(dolbyio::comms::video_frame& VideoFrame)
	{
#if PLATFORM_WINDOWS
		if (dolbyio::comms::video_frame_i420* FrameI420 = VideoFrame.get_i420_frame())
		{
			dolbyio::comms::video_utils::format_converter::i420_to_argb(
			    FrameI420->get_y(), FrameI420->stride_y(), FrameI420->get_u(), FrameI420->stride_u(),
			    FrameI420->get_v(), FrameI420->stride_v(), Buffer.GetData(), VideoFrame.width() * Stride,
			    VideoFrame.width(), VideoFrame.height());
		}
#elif PLATFORM_MAC
		class FLockedCVPixelBuffer
		{
		public:
			FLockedCVPixelBuffer(CVPixelBufferRef PixelBuffer) : PixelBuffer(PixelBuffer)
			{
				CVPixelBufferLockBaseAddress(PixelBuffer, kCVPixelBufferLock_ReadOnly);
			}
			~FLockedCVPixelBuffer()
			{
				CVPixelBufferUnlockBaseAddress(PixelBuffer, kCVPixelBufferLock_ReadOnly);
			}

			operator CVPixelBufferRef() const
			{
				return PixelBuffer;
			}

		private:
			CVPixelBufferRef PixelBuffer;
		};

		if (dolbyio::comms::video_frame_macos* FrameMac = VideoFrame.get_native_frame())
		{
			FLockedCVPixelBuffer PixelBuffer{FrameMac->get_buffer()};
			dolbyio::comms::video_utils::format_converter::nv12_to_argb(
			    static_cast<uint8*>(CVPixelBufferGetBaseAddressOfPlane(PixelBuffer, 0)),
			    CVPixelBufferGetBytesPerRowOfPlane(PixelBuffer, 0),
			    static_cast<uint8*>(CVPixelBufferGetBaseAddressOfPlane(PixelBuffer, 1)),
			    CVPixelBufferGetBytesPerRowOfPlane(PixelBuffer, 1), Buffer.GetData(), VideoFrame.width() * Stride,
			    VideoFrame.width(), VideoFrame.height());
		}
#endif
	}

	void FVideoSink::handle_frame(std::unique_ptr<dolbyio::comms::video_frame> VideoFrame)
	{
		AsyncTask(ENamedThreads::GameThread,
		          [WeakThis = weak_from_this(), VideoFrame = MoveTemp(VideoFrame)]
		          {
			          if (std::shared_ptr<FVideoSink> SharedThis = WeakThis.lock())
			          {
				          SharedThis->RecreateIfNeeded(VideoFrame->width(), VideoFrame->height());
				          SharedThis->Convert(*VideoFrame);
				          SharedThis->Fence.Wait();
				          SharedThis->Texture->UpdateTextureRegions(0, 1, &SharedThis->Region,
				                                                    VideoFrame->width() * Stride, Stride,
				                                                    SharedThis->Buffer.GetData());
				          SharedThis->Fence.Wait();
			          }
		          });
	}

	void FVideoSink::UpdateMaterial(UMaterialInstanceDynamic* Material)
	{
		if (IsValid(Material))
		{
			Material->SetTextureParameterValue("DolbyIO Frame", Texture);
		}
	}
}
