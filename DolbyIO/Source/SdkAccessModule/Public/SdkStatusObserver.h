// Copyright 2022 Dolby Laboratories

#pragma once

#include "Internationalization/Text.h"

namespace Dolby
{
	class ISdkStatusObserver
	{
	public:
		using FMessage = FString;
		using FDeviceName = FText;
		using FDeviceNames = TArray<FDeviceName>;

		virtual ~ISdkStatusObserver() = default;

		virtual void OnStatusChanged(const FMessage&) = 0;

		virtual void OnNewListOfInputDevices(const FDeviceNames&) = 0;
		virtual void OnNewListOfOutputDevices(const FDeviceNames&) = 0;
		virtual void OnInputDeviceChanged(const FDeviceName&) = 0;
		virtual void OnOutputDeviceChanged(const FDeviceName&) = 0;

		virtual void OnRefreshTokenRequested() = 0;
	};
}
