#pragma once

namespace Dolby
{
	class FSdkStatus;

	struct FExceptionHandler
	{
		FExceptionHandler(FSdkStatus& Status);

		void operator()(std::exception_ptr&& ExcPtr);
		template <typename Callee> void NotifyIfThrows(Callee function);
		void RethrowAndUpdateStatus();

	private:
		FSdkStatus& Status;
	};
}
