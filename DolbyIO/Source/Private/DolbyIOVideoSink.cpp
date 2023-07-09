// Copyright 2023 Dolby Laboratories

#include "DolbyIOVideoSink.h"

#include "DolbyIOLogging.h"

#include <dolbyio/comms/media_engine/video_utils.h>

#include "Async/Async.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "RenderingThread.h"
#include "Runtime/Launch/Resources/Version.h"
#include "TextureResource.h"

namespace DolbyIO
{
	using namespace dolbyio::comms;

	namespace
	{
		constexpr auto TexParamName = "DolbyIO Frame";
		constexpr int Stride = 4;

		class FLockedTexture
		{
		public:
			FLockedTexture(UTexture2D& Tex)
			    : Tex(Tex), PlatformData(*Tex.
#if ENGINE_MAJOR_VERSION == 4
			                              PlatformData
#else
			                              GetPlatformData()
#endif
			                             ),
			      Mip(PlatformData.Mips[0]), Buffer(Mip.BulkData.Lock(LOCK_READ_WRITE))
			{
			}

			~FLockedTexture()
			{
				Mip.BulkData.Unlock();
				Tex.UpdateResource();
				FlushRenderingCommands();
			}

			void Resize(int Width, int Height)
			{
				Mip.SizeX = PlatformData.SizeX = Width;
				Mip.SizeY = PlatformData.SizeY = Height;
				Buffer = Mip.BulkData.Realloc(Width * Height * Stride);
			}

			operator uint8*()
			{
				return static_cast<uint8*>(Buffer);
			}

		private:
			UTexture2D& Tex;
			FTexturePlatformData& PlatformData;
			FTexture2DMipMap& Mip;
			void* Buffer;
		};

		UTexture2D* CreateEmptyTexture()
		{
			UTexture2D* Ret = UTexture2D::CreateTransient(1, 1, EPixelFormat::PF_B8G8R8A8);
			Ret->AddToRoot();
			AsyncTask(ENamedThreads::GameThread,
			          [Ret]
			          {
				          FLockedTexture Tex{*Ret};
				          FMemory::Memzero(Tex, Stride);
			          });
			return Ret;
		}

		void UnbindMaterialImpl(UMaterialInstanceDynamic& Material)
		{
			static UTexture2D* EmptyTexture = CreateEmptyTexture();
			Material.SetTextureParameterValue(TexParamName, EmptyTexture);
		}
	}

	FVideoSink::FVideoSink(const FString& VideoTrackID) : Texture(CreateEmptyTexture()), VideoTrackID(VideoTrackID) {}

	FVideoSink::~FVideoSink()
	{
		Texture->RemoveFromRoot();
	}

	UTexture2D* FVideoSink::GetTexture()
	{
		return Texture;
	}

	void FVideoSink::BindMaterial(UMaterialInstanceDynamic* Material)
	{
		if (IsValid(Material))
		{
			DLB_UE_LOG("Binding material %u to video track ID %s texture %u", Material->GetUniqueID(), *VideoTrackID,
			           Texture->GetUniqueID());
			Materials.Add(Material);
			Material->SetTextureParameterValue(TexParamName, Texture);
		}
	}

	void FVideoSink::UnbindMaterial(UMaterialInstanceDynamic* Material)
	{
		if (Materials.Remove(Material) && IsValid(Material))
		{
			DLB_UE_LOG("Unbinding material %u from video track ID %s texture %u", Material->GetUniqueID(),
			           *VideoTrackID, Texture->GetUniqueID());
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
		Convert(VideoFrame);

		AsyncTask(ENamedThreads::GameThread,
		          [WeakThis = weak_from_this(), Frame = VideoFrame]
		          {
			          if (std::shared_ptr<FVideoSink> SharedThis = WeakThis.lock())
			          {
				          SharedThis->Render(Frame);
			          }
		          });
	}

	void FVideoSink::Render(const video_frame& VideoFrame)
	{
		const int Width = VideoFrame.width();
		const int Height = VideoFrame.height();

		if (Texture->GetSizeX() != Width || Texture->GetSizeY() != Height)
		{
			DLB_UE_LOG("Resizing texture %u for video track ID %s: old %dx%d new %dx%d", Texture->GetUniqueID(),
			           *VideoTrackID, Texture->GetSizeX(), Texture->GetSizeY(), Width, Height);
			FLockedTexture Tex{*Texture};
			Tex.Resize(Width, Height);
		}

		ENQUEUE_RENDER_COMMAND(DolbyIOUpdateTexture)
		(
		    [WeakThis = weak_from_this(), Width = static_cast<uint32>(Width),
		     Height = static_cast<uint32>(Height)](FRHICommandListImmediate& RHICmdList)
		    {
			    if (std::shared_ptr<FVideoSink> SharedThis = WeakThis.lock())
			    {
				    FScopeLock Lock{&SharedThis->BufferLock};
				    RHIUpdateTexture2D(SharedThis->Texture->GetResource()->GetTexture2DRHI(), 0,
				                       FUpdateTextureRegion2D{0, 0, 0, 0, Width, Height}, Width * Stride,
				                       SharedThis->Buffer.GetData());
			    }
		    });
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

		FScopeLock Lock{&BufferLock};
		Buffer.Reserve(Width * Height * Stride);

		enum video_frame_buffer::type VideoFrameBufferType = VideoFrameBuffer->type();

		if (VideoFrameBufferType == video_frame_buffer::type::argb)
		{
			if (const video_frame_buffer_argb_interface* FrameARGB = VideoFrameBuffer->get_argb())
			{
				video_utils::format_converter::argb_copy(FrameARGB->data(), FrameARGB->stride(), Buffer.GetData(),
				                                         Width * Stride, Width, Height);
			}
		}
		else if (VideoFrameBufferType == video_frame_buffer::type::i420)
		{
			if (const video_frame_buffer_i420_interface* FrameI420 = VideoFrameBuffer->get_i420())
			{
				video_utils::format_converter::i420_to_argb(
				    FrameI420->data_y(), FrameI420->stride_y(), FrameI420->data_u(), FrameI420->stride_u(),
				    FrameI420->data_v(), FrameI420->stride_v(), Buffer.GetData(), Width * Stride, Width, Height);
			}
		}
		else if (VideoFrameBufferType == video_frame_buffer::type::nv12)
		{
			if (const video_frame_buffer_nv12_interface* FrameNV12 = VideoFrameBuffer->get_nv12())
			{
				video_utils::format_converter::nv12_to_argb(FrameNV12->data_y(), FrameNV12->stride_y(),
				                                            FrameNV12->data_uv(), FrameNV12->stride_uv(),
				                                            Buffer.GetData(), Width * Stride, Width, Height);
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
				    CVPixelBufferGetBytesPerRowOfPlane(PixelBuffer, 1), Buffer.GetData(), Width * Stride, Width,
				    Height);
			}
		}
#endif
	}
}
