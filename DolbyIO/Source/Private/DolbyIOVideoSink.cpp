// Copyright 2023 Dolby Laboratories

#include "DolbyIOVideoSink.h"

#include "DolbyIOLogging.h"

#include <dolbyio/comms/media_engine/video_utils.h>

#include "Async/Async.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Runtime/Launch/Resources/Version.h"
#include "TextureResource.h"

namespace DolbyIO
{
	using namespace dolbyio::comms;

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
		}
		Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
		Texture->AddToRoot();

		for (UMaterialInstanceDynamic* Material : Materials)
		{
			UpdateMaterial(Material);
		}
	}

#if ENGINE_MAJOR_VERSION == 4
#define PLATFORM_DATA PlatformData
#else
#define PLATFORM_DATA GetPlatformData()
#endif

	void FVideoSink::Convert(const video_frame& VideoFrame)
	{
		constexpr int Stride = 4;

		class FTextureBuffer
		{
		public:
			FTextureBuffer(UTexture2D* Texture)
			    : Texture(Texture),
			      Buffer(reinterpret_cast<uint8_t*>(Texture->PLATFORM_DATA->Mips[0].BulkData.Lock(LOCK_READ_WRITE)))
			{
			}
			~FTextureBuffer()
			{
				Texture->PLATFORM_DATA->Mips[0].BulkData.Unlock();
				Texture->UpdateResource();
			}
			operator uint8_t*()
			{
				return Buffer;
			}

		private:
			UTexture2D* const Texture;
			uint8_t* const Buffer;
		};

		std::shared_ptr<video_frame_buffer> VideoFrameBuffer = VideoFrame.video_frame_buffer();

		if (!VideoFrameBuffer)
		{
			return;
		}

		enum video_frame_buffer::type VideoFrameBufferType = VideoFrameBuffer->type();

		if (VideoFrameBufferType == video_frame_buffer::type::argb)
		{
			if (const video_frame_buffer_argb_interface* FrameARGB = VideoFrameBuffer->get_argb())
			{
				video_utils::format_converter::argb_copy(FrameARGB->data(), FrameARGB->stride(),
				                                         FTextureBuffer{Texture}, VideoFrame.width() * Stride,
				                                         VideoFrame.width(), VideoFrame.height());
			}
		}
		else if (VideoFrameBufferType == video_frame_buffer::type::i420)
		{
			if (const video_frame_buffer_i420_interface* FrameI420 = VideoFrameBuffer->get_i420())
			{
				video_utils::format_converter::i420_to_argb(
				    FrameI420->data_y(), FrameI420->stride_y(), FrameI420->data_u(), FrameI420->stride_u(),
				    FrameI420->data_v(), FrameI420->stride_v(), FTextureBuffer{Texture}, VideoFrame.width() * Stride,
				    VideoFrame.width(), VideoFrame.height());
			}
		}
		else if (VideoFrameBufferType == video_frame_buffer::type::nv12)
		{
			if (const video_frame_buffer_nv12_interface* FrameNV12 = VideoFrameBuffer->get_nv12())
			{
				video_utils::format_converter::nv12_to_argb(
				    FrameNV12->data_y(), FrameNV12->stride_y(), FrameNV12->data_uv(), FrameNV12->stride_uv(),
				    FTextureBuffer{Texture}, VideoFrame.width() * Stride, VideoFrame.width(), VideoFrame.height());
			}
		}
#if PLATFORM_MAC
		else if (VideoFrameBufferType == video_frame_buffer::type::native)
		{
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

			if (const video_frame_buffer_native_interface* FrameNative = VideoFrameBuffer->get_native())
			{
				FLockedCVPixelBuffer PixelBuffer{FrameNative->cv_pixel_buffer_ref()};
				video_utils::format_converter::nv12_to_argb(
				    static_cast<uint8*>(CVPixelBufferGetBaseAddressOfPlane(PixelBuffer, 0)),
				    CVPixelBufferGetBytesPerRowOfPlane(PixelBuffer, 0),
				    static_cast<uint8*>(CVPixelBufferGetBaseAddressOfPlane(PixelBuffer, 1)),
				    CVPixelBufferGetBytesPerRowOfPlane(PixelBuffer, 1), FTextureBuffer{Texture},
				    VideoFrame.width() * Stride, VideoFrame.width(), VideoFrame.height());
			}
		}
#endif
		else
		{
			DLB_UE_WARN("Unsupported video frame buffer type %d", VideoFrameBufferType);
		}
	}

	void FVideoSink::handle_frame(const video_frame& VideoFrame)
	{
		AsyncTask(ENamedThreads::GameThread,
		          [WeakThis = weak_from_this(), Frame = VideoFrame]
		          {
			          if (std::shared_ptr<FVideoSink> SharedThis = WeakThis.lock())
			          {
				          SharedThis->RecreateIfNeeded(Frame.width(), Frame.height());
				          SharedThis->Convert(Frame);
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
