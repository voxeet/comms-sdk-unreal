// Copyright 2023 Dolby Laboratories

#include "DolbyIOErrorHandler.h"

#include <dolbyio/comms/sdk_exceptions.h>

namespace DolbyIO
{
	using namespace dolbyio::comms;

	FErrorHandler::FErrorHandler(FErrorHandlerImpl Impl) : Impl(Impl) {}

	void FErrorHandler::operator()(std::exception_ptr&& ExcPtr)
	{
		HandleError([ExcP = MoveTemp(ExcPtr)] { std::rethrow_exception(ExcP); });
	}

	void FErrorHandler::HandleError()
	{
		HandleError([] { throw; });
	}

	void FErrorHandler::HandleError(TFunction<void()> Callee)
	try
	{
		Callee();
	}
	catch (const conference_state_exception& Ex)
	{
		Impl(FString{"Caught dolbyio::comms::conference_state_exception: "} + Ex.what());
	}
	catch (const invalid_token_exception& Ex)
	{
		Impl(FString{"Caught dolbyio::comms::invalid_token_exception: "} + Ex.what());
	}
	catch (const dvc_error_exception& Ex)
	{
		Impl(FString{"Caught dolbyio::comms::dvc_error_exception: "} + Ex.what());
	}
	catch (const peer_connection_failed_exception& Ex)
	{
		Impl(FString{"Caught dolbyio::comms::peer_connection_failed_exception: "} + Ex.what());
	}
	catch (const dolbyio::comms::exception& Ex)
	{
		Impl(FString{"Caught dolbyio::comms::exception: "} + Ex.what());
	}
	catch (const std::exception& Ex)
	{
		Impl(FString{"Caught std::exception: "} + Ex.what());
	}
	catch (...)
	{
		Impl("Caught unknown exception");
	}
}
