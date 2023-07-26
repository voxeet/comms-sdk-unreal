// Copyright 2023 Dolby Laboratories

#pragma once

#include "Utils/DolbyIOCppSdk.h"

#include "Containers/UnrealString.h"
#include "DolbyIOTypes.h"
#include "Misc/Optional.h"

class UDolbyIOSubsystem;

namespace DolbyIO
{
	class FDevices
	{
		using FDeviceManagement = dolbyio::comms::services::device_management;

	public:
		FDevices(UDolbyIOSubsystem& Subsystem, FDeviceManagement& DeviceManagement);

		void GetAudioInputDevices();
		void GetAudioOutputDevices();
		void GetCurrentAudioInputDevice();
		void GetCurrentAudioOutputDevice();
		void SetAudioInputDevice(const FString& NativeID);
		void SetAudioOutputDevice(const FString& NativeID);
		void GetVideoDevices();
		void GetCurrentVideoDevice();

		UDolbyIOSubsystem& GetSubsystem()
		{
			return Subsystem;
		}

	private:
		UDolbyIOSubsystem& Subsystem;
		FDeviceManagement& DeviceManagement;
	};
}
