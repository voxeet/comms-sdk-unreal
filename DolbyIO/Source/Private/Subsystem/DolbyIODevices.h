// Copyright 2023 Dolby Laboratories

#pragma once

#include "Utils/DolbyIOCppSdk.h"

#include "Containers/UnrealString.h"

class UDolbyIOSubsystem;

namespace DolbyIO
{
	class FDevices
	{
		using FDeviceManagement = dolbyio::comms::services::device_management;

	public:
		FDevices(UDolbyIOSubsystem& Subsystem, FDeviceManagement& DeviceManagement);

		dolbyio::comms::async_result<dolbyio::comms::event_handler_id> RegisterDeviceEventHandlers();

		void GetAudioInputDevices();
		void GetAudioOutputDevices();
		void GetCurrentAudioInputDevice();
		void GetCurrentAudioOutputDevice();
		void SetAudioInputDevice(const FString& NativeID);
		void SetAudioOutputDevice(const FString& NativeID);
		void GetVideoDevices();

	private:
		UDolbyIOSubsystem& Subsystem;
		FDeviceManagement& DeviceManagement;
	};
}
