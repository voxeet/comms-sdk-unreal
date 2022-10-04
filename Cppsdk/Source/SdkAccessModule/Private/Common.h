#pragma once

#define DOLBYIO_COMMS_SUPPRESS_APPLE_NO_RTTI_WARNING
#include <dolbyio/comms/sdk.h>

namespace Dolby
{

	inline auto ToFText(const std::string& String)
	{
		return FText::FromString(UTF8_TO_TCHAR(String.c_str()));
	}

}
