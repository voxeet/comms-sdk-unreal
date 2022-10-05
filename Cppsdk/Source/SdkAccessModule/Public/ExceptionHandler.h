#pragma once

namespace Dolby
{
	class FSdkStatus;

	struct FExceptionHandler
	{
		FExceptionHandler(FSdkStatus& Status);

		void operator()(std::exception_ptr&& ExcPtr);
		void NotifyIfThrows(std::function<void()> function);

	private:
		FSdkStatus& Status;
	};
}
