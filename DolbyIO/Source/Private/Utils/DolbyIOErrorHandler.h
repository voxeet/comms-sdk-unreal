// Copyright 2023 Dolby Laboratories

#pragma once

#include "Templates/Function.h"

class UDolbyIOSubsystem;

namespace DolbyIO
{
	class FErrorHandler final
	{
	public:
#define DLB_ERROR_HANDLER_BASE(Subsystem) FErrorHandler(Subsystem, __LINE__)
#define DLB_ERROR_HANDLER DLB_ERROR_HANDLER_BASE(*this)
#define DLB_SUBSYSTEM_ERROR_HANDLER DLB_ERROR_HANDLER_BASE(Subsystem)
		FErrorHandler(UDolbyIOSubsystem& DolbyIOSubsystem, int Line);

		void operator()(std::exception_ptr&& ExcPtr) const;
		void HandleError() const;

	private:
		void HandleError(TFunction<void()> Callee) const;
		void LogException(const FString& Type, const FString& What) const;

		UDolbyIOSubsystem& DolbyIOSubsystem;
		int Line;
	};
}
