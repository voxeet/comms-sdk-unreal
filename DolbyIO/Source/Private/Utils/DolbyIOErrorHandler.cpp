// Copyright 2023 Dolby Laboratories

#include "Utils/DolbyIOErrorHandler.h"

#include "DolbyIO.h"
#include "Utils/DolbyIOConversions.h"
#include "Utils/DolbyIOLogging.h"

using namespace dolbyio::comms;
using namespace DolbyIO;

namespace DolbyIO
{
	FErrorHandler::FErrorHandler(UDolbyIOSubsystem& DolbyIOSubsystem, int Line)
	    : DolbyIOSubsystem(DolbyIOSubsystem), Line(Line)
	{
	}

	void FErrorHandler::operator()(std::exception_ptr&& ExcPtr) const
	{
		HandleError([ExcP = MoveTemp(ExcPtr)] { std::rethrow_exception(ExcP); });
	}

	void FErrorHandler::HandleError() const
	{
		HandleError([] { throw; });
	}

	void FErrorHandler::HandleError(TFunction<void()> Callee) const
	try
	{
		Callee();
	}
	catch (const conference_state_exception& Ex)
	{
		LogException("dolbyio::comms::conference_state_exception", Ex.what());
	}
	catch (const invalid_token_exception& Ex)
	{
		LogException("dolbyio::comms::invalid_token_exception", Ex.what());
	}
	catch (const dvc_error_exception& Ex)
	{
		LogException("dolbyio::comms::dvc_error_exception", Ex.what());
	}
	catch (const peer_connection_failed_exception& Ex)
	{
		LogException("dolbyio::comms::peer_connection_failed_exception", Ex.what());
	}
	catch (const dolbyio::comms::exception& Ex)
	{
		LogException("dolbyio::comms::exception", Ex.what());
	}
	catch (const std::exception& Ex)
	{
		LogException("std::exception", Ex.what());
	}
	catch (...)
	{
		LogException("unknown exception", "");
	}

	void FErrorHandler::LogException(const FString& Type, const FString& What) const
	{
		DLB_UE_ERROR("Caught %s: %s (conference status: %s, line: %d)", *Type, *What,
		             *ToString(DolbyIOSubsystem.ConferenceStatus), Line);
	}
}
