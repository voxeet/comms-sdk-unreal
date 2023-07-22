// Copyright 2023 Dolby Laboratories

#include "Utils/DolbyIOErrorHandler.h"

#include "DolbyIO.h"
#include "Utils/DolbyIOBroadcastEvent.h"
#include "Utils/DolbyIOConversions.h"
#include "Utils/DolbyIOLogging.h"

#include "Misc/Paths.h"

namespace DolbyIO
{
	using namespace dolbyio::comms;

	FErrorHandler::FErrorHandler(const FString& File, int Line, UDolbyIOSubsystem& DolbyIOSubsystem)
	    : File(FPaths::GetCleanFilename(File)), Line(Line), DolbyIOSubsystem(DolbyIOSubsystem)
	{
	}

	FErrorHandler::FErrorHandler(const FString& File, int Line, UDolbyIOSubsystem& DolbyIOSubsystem,
	                             const FDolbyIOOnErrorDelegate& OnError)
	    : File(FPaths::GetCleanFilename(File)), Line(Line), DolbyIOSubsystem(DolbyIOSubsystem), OnError(&OnError)
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
		const FString ErrorMsg = Type + ": " + What;
		DLB_UE_LOG_BASE(Error, "Caught %s (conference status: %s, %s:%d)", *ErrorMsg,
		                *ToString(DolbyIOSubsystem.ConferenceStatus), *File, Line);
		if (OnError)
		{
			BroadcastEvent(*OnError, ErrorMsg);
		}
	}

	void FErrorHandler::Warn(const FDolbyIOOnErrorDelegate& OnError, const FString& Msg)
	{
		DLB_UE_LOG_BASE(Warning, "%s", *Msg);
		BroadcastEvent(OnError, Msg);
	}
}
