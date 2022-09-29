#pragma once

#define DOLBYIO_COMMS_SUPPRESS_APPLE_NO_RTTI_WARNING
#include <dolbyio/comms/sdk.h>

#include "SdkStatus.h"
#include "SdkStatusObserver.h"

#define DLB_CATCH_ALL                                                                          \
	catch (const dolbyio::comms::exception& Ex)                                                \
	{                                                                                          \
		Status.SetMsg(FSdkStatus::FMessage{"Caught dolbyio::comms::exception: "} + Ex.what()); \
	}                                                                                          \
	catch (const std::exception& Ex)                                                           \
	{                                                                                          \
		Status.SetMsg(FSdkStatus::FMessage{"Caught std::exception: "} + Ex.what());            \
	}                                                                                          \
	catch (...)                                                                                \
	{                                                                                          \
		Status.SetMsg("Caught unknown exception");                                             \
	}

namespace Dolby
{
	inline void HandleAsyncException(const std::exception_ptr& ExPtr, FSdkStatus& Status)
	try
	{
		std::rethrow_exception(ExPtr);
	}
	DLB_CATCH_ALL

	inline auto ToFText(const std::string& String)
	{
		return FText::FromString(UTF8_TO_TCHAR(String.c_str()));
	}
}

#define DLB_HANDLE_ASYNC_EXCEPTION [this](auto&& ExPtr) { Dolby::HandleAsyncException(ExPtr, Status); }
