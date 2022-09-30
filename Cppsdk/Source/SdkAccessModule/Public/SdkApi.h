#pragma once

namespace Dolby
{
	class ISdkApi
	{
	public:
		using FMessage = FString;
		using FDeviceName = FText;
		using FDeviceNames = TArray<FDeviceName>;

		virtual ~ISdkApi() = default;

		virtual void OnStatusChanged(const FMessage&) = 0;

		virtual void OnNewListOfInputDevices(const FDeviceNames&) = 0;
		virtual void OnNewListOfOutputDevices(const FDeviceNames&) = 0;
		virtual void OnInputDeviceChanged(const FDeviceName&) = 0;
		virtual void OnOutputDeviceChanged(const FDeviceName&) = 0;

		virtual void OnRefreshTokenRequested() = 0;
	};
}
