#pragma once

namespace Dolby
{
	using FMessage = FString;

	struct FExceptionHandler
	{
		using FNotifyingFunction = std::function<void(const FMessage&)>;

		FExceptionHandler(FNotifyingFunction&& LogFunction);

		void operator()(std::exception_ptr&& ExcPtr);
		void NotifyIfThrows(std::function<void()> function);

	private:
		FNotifyingFunction Notify;
	};
}
