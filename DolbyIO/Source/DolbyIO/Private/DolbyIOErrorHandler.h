// Copyright 2023 Dolby Laboratories

#pragma once

#include "Containers/UnrealString.h"

namespace DolbyIO
{
	class FErrorHandler final
	{
		using FErrorHandlerImpl = TFunction<void(const FString&)>;

	public:
		FErrorHandler(FErrorHandlerImpl);

		void operator()(class std::exception_ptr&&);
		void HandleError();

	private:
		void HandleError(TFunction<void()>);

		FErrorHandlerImpl Impl;
	};
}
