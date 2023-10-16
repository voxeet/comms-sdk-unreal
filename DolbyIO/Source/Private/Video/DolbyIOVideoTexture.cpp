// Copyright 2023 Dolby Laboratories

#include "DolbyIOVideoTexture.h"

#include "Engine/Texture2D.h"
#include "RenderingThread.h"
#include "Runtime/Launch/Resources/Version.h"
#include "TextureResource.h"

namespace DolbyIO
{
	FVideoTexture::FVideoTexture(int Width, int Height) : Texture(UTexture2D::CreateTransient(Width, Height))
	{
		Texture->AddToRoot();
		Texture->UpdateResource();
		Resize(Width, Height);
	}

	FVideoTexture::~FVideoTexture()
	{
		Texture->RemoveFromRoot();
	}

	UTexture2D* FVideoTexture::GetTexture()
	{
		return Texture;
	}

	bool FVideoTexture::Resize(int InWidth, int InHeight)
	{
		FScopeLock Lock{&BufferLock};

		if (Width == InWidth && Height == InHeight)
		{
			return false;
		}

		Width = InWidth;
		Height = InHeight;
		Buffer.Reserve(Width * Height * Stride);
		return true;
	}

	FCriticalSection* FVideoTexture::GetBufferLock()
	{
		return &BufferLock;
	}

	uint8* FVideoTexture::GetBuffer()
	{
		return Buffer.GetData();
	}

	namespace
	{
#if ENGINE_MAJOR_VERSION == 5
#define PLATFORM_DATA GetPlatformData()
#else
#define PLATFORM_DATA PlatformData
#endif

		class FLockedTexture
		{
		public:
			FLockedTexture(UTexture2D& Tex)
			    : Tex(Tex), PlatformData(*Tex.PLATFORM_DATA), Mip(PlatformData.Mips[0]),
			      Buffer(Mip.BulkData.Lock(LOCK_READ_WRITE))
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
				Buffer = Mip.BulkData.Realloc(Width * Height * FVideoTexture::Stride);
				Clear();
			}

			void Clear()
			{
				FMemory::Memzero(Buffer, Mip.BulkData.GetBulkDataSize());
			}

		private:
			UTexture2D& Tex;
			FTexturePlatformData& PlatformData;
			FTexture2DMipMap& Mip;
			void* Buffer;
		};
	}

	void FVideoTexture::Render()
	{
		if (Texture->GetSizeX() != Width || Texture->GetSizeY() != Height)
		{
			FLockedTexture Tex{*Texture};
			Tex.Resize(Width, Height);
		}

		ENQUEUE_RENDER_COMMAND(DolbyIOUpdateTexture)
		(
		    [SharedThis = AsShared()](FRHICommandListImmediate& RHICmdList)
		    {
			    FScopeLock Lock{SharedThis->GetBufferLock()};

				const auto TextureRegion = FUpdateTextureRegion2D{0, 0, 0, 0,
																	static_cast<uint32>(SharedThis->Texture->GetResource()->GetTexture2DRHI()->GetSizeX()),
																	static_cast<uint32>(SharedThis->Texture->GetResource()->GetTexture2DRHI()->GetSizeY())};

				if (TextureRegion.DestX + TextureRegion.Width <= static_cast<uint32>(SharedThis->Texture->GetResource()->GetTexture2DRHI()->GetSizeX()) &&
					TextureRegion.DestY + TextureRegion.Height <= static_cast<uint32>(SharedThis->Texture->GetResource()->GetTexture2DRHI()->GetSizeY()))
				{
					RHIUpdateTexture2D(SharedThis->Texture->GetResource()->GetTexture2DRHI(), 
										0,
										TextureRegion,
										SharedThis->Texture->GetResource()->GetTexture2DRHI()->GetSizeX() * Stride,
										SharedThis->GetBuffer());
				}
			});
	}

	namespace
	{
		UTexture2D* CreateEmptyTexture()
		{
			UTexture2D* Ret = UTexture2D::CreateTransient(1, 1);
			Ret->AddToRoot();
			FLockedTexture Tex{*Ret};
			Tex.Clear();
			return Ret;
		}
	}

	UTexture2D* FVideoTexture::GetEmptyTexture()
	{
		static UTexture2D* Ret = CreateEmptyTexture();
		return Ret;
	}
}
