// Copyright 2023 Dolby Laboratories

#pragma once

#include "Templates/Function.h"

class UDolbyIOSubsystem;
class FDolbyIOOnErrorDelegate;

namespace DolbyIO
{
	class FErrorHandler final
	{
	public:
#define DLB_ERROR_HANDLER(OnError) FErrorHandler(__LINE__, GetSubsystem(), OnError)
#define DLB_ERROR_HANDLER_NO_DELEGATE FErrorHandler(__LINE__, GetSubsystem())

#define DLB_WARNING(OnError, Msg) FErrorHandler::Warn(OnError, Msg)

		FErrorHandler(int Line, UDolbyIOSubsystem& DolbyIOSubsystem);
		FErrorHandler(int Line, UDolbyIOSubsystem& DolbyIOSubsystem, const FDolbyIOOnErrorDelegate& OnError);

		void operator()(std::exception_ptr&& ExcPtr) const;
		void HandleError() const;

		static void Warn(const FDolbyIOOnErrorDelegate& OnError, const FString& Msg);

	private:
		void HandleError(TFunction<void()> Callee) const;
		void LogException(const FString& Type, const FString& What) const;

		int Line;
		UDolbyIOSubsystem& DolbyIOSubsystem;
		const FDolbyIOOnErrorDelegate* const OnError{};
	};
}
