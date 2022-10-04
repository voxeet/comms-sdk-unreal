#include "ExceptionHandler.h"

namespace Dolby
{
	FExceptionHandler::FExceptionHandler(FNotifyingFunction&& NotifyingFunction) : Notify(NotifyingFunction) {}

	void FExceptionHandler::operator()(std::exception_ptr&& ExcPtr)
	{
		NotifyIfThrows([ExcPtr]() { std::rethrow_exception(ExcPtr); });
	}

	void FExceptionHandler::NotifyIfThrows(std::function<void()> function)
	try
	{
		function();
	}
	catch (const dolbyio::comms::exception& Ex)
	{
		Notify(FMessage{"Caught dolbyio::comms::exception: "} + Ex.what());
	}
	catch (const std::exception& Ex)
	{
		Notify(FMessage{"Caught std::exception: "} + Ex.what());
	}
	catch (...)
	{
		Notify("Caught unknown exception");
	}
}
