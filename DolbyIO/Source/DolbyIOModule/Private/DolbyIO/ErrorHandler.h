// Copyright 2022 Dolby Laboratories

#pragma once

#include "Containers/UnrealString.h"

namespace DolbyIO
{
	class FErrorHandler
	{
		using FStatusUpdater = TFunction<void(const FString&)>;
		using FDisconnector = TFunction<void()>;

	public:
		FErrorHandler(FStatusUpdater, FDisconnector);

		void operator()(class std::exception_ptr&&);
		void RethrowAndUpdateStatus();

	private:
		void NotifyIfThrows(TFunction<void()>);

		FStatusUpdater UpdateStatus;
		FDisconnector Disconnect;
	};
}
