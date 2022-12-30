#include "DolbyIO/ErrorHandler.h"

#include <dolbyio/comms/sdk_exceptions.h>

namespace DolbyIO
{
	using namespace dolbyio::comms;

	FErrorHandler::FErrorHandler(FStatusUpdater UpdateStatus) : UpdateStatus(UpdateStatus) {}

	void FErrorHandler::operator()(std::exception_ptr&& ExcPtr)
	{
		NotifyIfThrows([ExcP = MoveTemp(ExcPtr)] { std::rethrow_exception(ExcP); });
	}

	void FErrorHandler::RethrowAndUpdateStatus()
	{
		NotifyIfThrows([] { throw; });
	}

	void FErrorHandler::NotifyIfThrows(TFunction<void()> Callee)
	try
	{
		Callee();
	}
	catch (const conference_state_exception& Ex)
	{
		UpdateStatus(FString{"Caught dolbyio::comms::conference_state_exception: "} + Ex.what());
	}
	catch (const invalid_token_exception& Ex)
	{
		UpdateStatus(FString{"Caught dolbyio::comms::invalid_token_exception: "} + Ex.what());
	}
	catch (const dvc_error_exception& Ex)
	{
		UpdateStatus(FString{"Caught dolbyio::comms::dvc_error_exception: "} + Ex.what());
	}
	catch (const peer_connection_failed_exception& Ex)
	{
		UpdateStatus(FString{"Caught dolbyio::comms::peer_connection_failed_exception: "} + Ex.what());
	}
	catch (const dolbyio::comms::exception& Ex)
	{
		UpdateStatus(FString{"Caught dolbyio::comms::exception: "} + Ex.what());
	}
	catch (const std::exception& Ex)
	{
		UpdateStatus(FString{"Caught std::exception: "} + Ex.what());
	}
	catch (...)
	{
		UpdateStatus("Caught unknown exception");
	}
}
