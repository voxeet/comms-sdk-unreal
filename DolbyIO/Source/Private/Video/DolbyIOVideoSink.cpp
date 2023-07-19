// Copyright 2023 Dolby Laboratories

#include "DolbyIOVideoSink.h"

#include "DolbyIOVideoTexture.h"
#include "Utils/DolbyIOLogging.h"

#include <dolbyio/comms/media_engine/video_utils.h>

#include "Async/Async.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace DolbyIO
{
	using namespace dolbyio::comms;

	namespace
	{
		constexpr auto TexParamName = "DolbyIO Frame";

		void UnbindMaterialImpl(UMaterialInstanceDynamic& Material)
		{
			Material.SetTextureParameterValue(TexParamName, FVideoTexture::GetEmptyTexture());
		}
	}

	FVideoSink::FVideoSink(const FString& VideoTrackID) : VideoTrackID(VideoTrackID) {}

	void FVideoSink::OnTextureCreated(FOnTextureCreated OnTextureCreated)
	{
		if (Texture)
		{
			return OnTextureCreated();
		}
		OnTexCreated = MoveTemp(OnTextureCreated);
	}

	UTexture2D* FVideoSink::GetTexture()
	{
		return Texture ? Texture->GetTexture() : nullptr;
	}

	void FVideoSink::BindMaterial(UMaterialInstanceDynamic* Material)
	{
		if (IsValid(Material))
		{
			DLB_UE_LOG("Binding material %u to video track ID %s", Material->GetUniqueID(), *VideoTrackID);
			Materials.Add(Material);
			if (Texture)
			{
				Material->SetTextureParameterValue(TexParamName, GetTexture());
			}
		}
	}

	void FVideoSink::UnbindMaterial(UMaterialInstanceDynamic* Material)
	{
		if (Materials.Remove(Material) && IsValid(Material))
		{
			DLB_UE_LOG("Unbinding material %u from video track ID %s", Material->GetUniqueID(), *VideoTrackID);
			UnbindMaterialImpl(*Material);
		}
	}

	void FVideoSink::UnbindAllMaterials()
	{
		AsyncTask(ENamedThreads::GameThread,
		          [MaterialsArray = Materials.Array()]
		          {
			          for (UMaterialInstanceDynamic* Material : MaterialsArray)
			          {
				          if (IsValid(Material))
				          {
					          UnbindMaterialImpl(*Material);
				          }
			          }
		          });
	}

	void FVideoSink::handle_frame(const video_frame& VideoFrame)
	{
		!Texture ? CreateTexture(VideoFrame.width(), VideoFrame.height())
		         : ResizeTexture(VideoFrame.width(), VideoFrame.height());
		Convert(VideoFrame);
		AsyncTask(ENamedThreads::GameThread, [Tex = this->Texture] { Tex->Render(); });
	}

	void FVideoSink::CreateTexture(int Width, int Height)
	{
		FEvent* TexCreated = FGenericPlatformProcess::GetSynchEventFromPool();
		AsyncTask(ENamedThreads::GameThread,
		          [=]
		          {
			          Texture = MakeShared<FVideoTexture>(Width, Height);
			          TexCreated->Trigger();

			          for (UMaterialInstanceDynamic* Material : Materials)
			          {
				          if (IsValid(Material))
				          {
					          Material->SetTextureParameterValue(TexParamName, GetTexture());
				          }
			          }
		          });
		TexCreated->Wait();
		FGenericPlatformProcess::ReturnSynchEventToPool(TexCreated);
		OnTexCreated();
		DLB_UE_LOG("Created texture %u for video track ID %s %dx%d", GetTexture()->GetUniqueID(), *VideoTrackID, Width,
		           Height);
	}

	void FVideoSink::ResizeTexture(int Width, int Height)
	{
		if (Texture->Resize(Width, Height))
		{
			AsyncTask(ENamedThreads::GameThread,
			          [=, Tex = this->Texture]
			          {
				          DLB_UE_LOG("Resizing texture %u: old %dx%d new %dx%d", Tex->GetTexture()->GetUniqueID(),
				                     Tex->GetTexture()->GetSizeX(), Tex->GetTexture()->GetSizeY(), Width, Height);
			          });
		}
	}

	void FVideoSink::Convert(const video_frame& VideoFrame)
	{
		std::shared_ptr<video_frame_buffer> VideoFrameBuffer = VideoFrame.video_frame_buffer();

		if (!VideoFrameBuffer)
		{
			return;
		}

		const int Width = VideoFrame.width();
		const int Height = VideoFrame.height();
		const int DestStride = Width * FVideoTexture::Stride;

		FScopeLock Lock{Texture->GetBufferLock()};

		enum video_frame_buffer::type VideoFrameBufferType = VideoFrameBuffer->type();

		if (VideoFrameBufferType == video_frame_buffer::type::argb)
		{
			if (const video_frame_buffer_argb_interface* FrameARGB = VideoFrameBuffer->get_argb())
			{
				video_utils::format_converter::argb_copy(FrameARGB->data(), FrameARGB->stride(), Texture->GetBuffer(),
				                                         DestStride, Width, Height);
			}
		}
		else if (VideoFrameBufferType == video_frame_buffer::type::i420)
		{
			if (const video_frame_buffer_i420_interface* FrameI420 = VideoFrameBuffer->get_i420())
			{
				video_utils::format_converter::i420_to_argb(
				    FrameI420->data_y(), FrameI420->stride_y(), FrameI420->data_u(), FrameI420->stride_u(),
				    FrameI420->data_v(), FrameI420->stride_v(), Texture->GetBuffer(), DestStride, Width, Height);
			}
		}
		else if (VideoFrameBufferType == video_frame_buffer::type::nv12)
		{
			if (const video_frame_buffer_nv12_interface* FrameNV12 = VideoFrameBuffer->get_nv12())
			{
				video_utils::format_converter::nv12_to_argb(FrameNV12->data_y(), FrameNV12->stride_y(),
				                                            FrameNV12->data_uv(), FrameNV12->stride_uv(),
				                                            Texture->GetBuffer(), DestStride, Width, Height);
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
				    CVPixelBufferGetBytesPerRowOfPlane(PixelBuffer, 1), Texture->GetBuffer(), DestStride, Width,
				    Height);
			}
		}
#endif
	}
}
