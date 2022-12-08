// Copyright 2022 Dolby Laboratories

#pragma once

#include "CommonTypes.h"

#include <exception>
#include <functional>

namespace Dolby
{
	using FStatusUpdater = std::function<void(const FMessage&)>;
	using FDisconnector = std::function<void()>;

	struct FErrorHandler
	{
		FErrorHandler(FStatusUpdater UpdateStatus, FDisconnector Disconnect);

		void operator()(std::exception_ptr&& ExcPtr);
		template <typename FCallee> void NotifyIfThrows(FCallee Callee);
		void RethrowAndUpdateStatus();

	private:
		FStatusUpdater UpdateStatus;
		FDisconnector Disconnect;
	};
}
