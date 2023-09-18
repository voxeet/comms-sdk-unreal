// Copyright 2023 Dolby Laboratories

#pragma once

#include "Containers/UnrealString.h"

class UDolbyIOSubsystem;
class FDolbyIOErrorDelegate;

namespace DolbyIO
{
	class FErrorHandler final
	{
	public:
#define DLB_ERROR_HANDLER(OnError) FErrorHandler(__FILE__, __LINE__, GetSubsystem(), OnError)
#define DLB_ERROR_HANDLER_NO_DELEGATE FErrorHandler(__FILE__, __LINE__, GetSubsystem())

#define DLB_WARNING(OnError, Msg) FErrorHandler::Warn(OnError, Msg)

		FErrorHandler(const FString& File, int Line, UDolbyIOSubsystem& DolbyIOSubsystem);
		FErrorHandler(const FString& File, int Line, UDolbyIOSubsystem& DolbyIOSubsystem,
		              const FDolbyIOErrorDelegate& OnError);

		void operator()(std::exception_ptr&& ExcPtr) const;
		void HandleError() const;

		static void Warn(const FDolbyIOErrorDelegate& OnError, const FString& Msg);

	private:
		void HandleError(TFunction<void()> Callee) const;
		void HandleErrorOnGameThread(std::exception_ptr&& ExcPtr) const;
		void LogException(const FString& Type, const FString& What) const;

		const FString File;
		const int Line;
		const UDolbyIOSubsystem& DolbyIOSubsystem;
		const FDolbyIOErrorDelegate* const OnError{};
	};
}
