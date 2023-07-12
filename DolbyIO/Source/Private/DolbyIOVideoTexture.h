// Copyright 2023 Dolby Laboratories

#pragma once

#include "HAL/CriticalSection.h"
#include "Templates/SharedPointer.h"

class UTexture2D;

namespace DolbyIO
{
	class FVideoTexture final : public TSharedFromThis<FVideoTexture>
	{
	public:
		FVideoTexture(int Width, int Height);
		~FVideoTexture();

		UTexture2D* GetTexture();

		bool Resize(int Width, int Height);
		FCriticalSection* GetBufferLock();
		uint8* GetBuffer();
		void Render();

		static UTexture2D* GetEmptyTexture();

		static constexpr int Stride = 4;

	private:
		UTexture2D* const Texture;
		TArray<uint8> Buffer;
		FCriticalSection BufferLock;
		int Width;
		int Height;
	};
}
