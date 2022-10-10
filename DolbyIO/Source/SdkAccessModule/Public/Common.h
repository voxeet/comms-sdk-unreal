#pragma once

#define DOLBYIO_COMMS_SUPPRESS_APPLE_NO_RTTI_WARNING
#include <dolbyio/comms/sdk.h>

namespace Dolby
{
	using FToken = FString;
	using FConferenceName = FString;
	using FUserName = FString;
	using FMessage = FString;
	using FDeviceName = FText;
	using FDeviceNames = TArray<FDeviceName>;

	inline auto ToFText(const std::string& String)
	{
		return FText::FromString(UTF8_TO_TCHAR(String.c_str()));
	}

}
