#include "DolbyIO/ErrorHandler.h"

#include <dolbyio/comms/sdk_exceptions.h>

namespace DolbyIO
{
	using namespace dolbyio::comms;

	FErrorHandler::FErrorHandler(FStatusUpdater UpdateStatus, FDisconnector Disconnect)
	    : UpdateStatus(UpdateStatus), Disconnect(Disconnect)
	{
	}

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
		auto constexpr NOT_JOINED = "3";
		UpdateStatus(FString{"Caught dolbyio::comms::conference_state_exception: "} + Ex.what());
		if (Ex.current_state() != NOT_JOINED)
		{
			Disconnect();
		}
	}
	catch (const invalid_token_exception& Ex)
	{
		UpdateStatus(FString{"Caught dolbyio::comms::invalid_token_exception: "} + Ex.what());
		Disconnect();
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
