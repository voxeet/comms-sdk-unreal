#include "ExceptionHandler.h"
#include "SdkStatus.h"

#include "dolbyio/comms/sdk_exceptions.h"

namespace Dolby
{
	FExceptionHandler::FExceptionHandler(FSdkStatus& Status) : Status(Status) {}

	void FExceptionHandler::operator()(std::exception_ptr&& ExcPtr)
	{
		NotifyIfThrows([ExcPtr]() { std::rethrow_exception(ExcPtr); });
	}

	template<typename Callee>
	void FExceptionHandler::NotifyIfThrows(Callee Callee)
	try
	{
		Callee();
	}
	catch (const dolbyio::comms::conference_state_exception& Ex)
	{
		Status.OnDisconnected();
		Status.SetMsg(FMessage{"Caught dolbyio::comms::conference_state_exception: "} + Ex.what());
	}
	catch (const dolbyio::comms::exception& Ex)
	{
		Status.SetMsg(FMessage{"Caught dolbyio::comms::exception: "} + Ex.what());
	}
	catch (const std::exception& Ex)
	{
		Status.SetMsg(FMessage{"Caught std::exception: "} + Ex.what());
	}
	catch (...)
	{
		Status.SetMsg("Caught unknown exception");
	}

	void FExceptionHandler::RethrowAndUpdateStatus()
	{
		NotifyIfThrows([]() { throw; });
	}
}
