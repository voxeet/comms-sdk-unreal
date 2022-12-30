// Copyright 2022 Dolby Laboratories

#pragma once

#include "Containers/UnrealString.h"

namespace DolbyIO
{
	class FErrorHandler final
	{
		using FStatusUpdater = TFunction<void(const FString&)>;

	public:
		FErrorHandler(FStatusUpdater);

		void operator()(class std::exception_ptr&&);
		void RethrowAndUpdateStatus();

	private:
		void NotifyIfThrows(TFunction<void()>);

		FStatusUpdater UpdateStatus;
	};
}
