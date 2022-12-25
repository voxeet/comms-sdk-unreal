// Copyright 2022 Dolby Laboratories

#pragma once

#include "DolbyIO/Typedefs.h"

#include "Templates/UniquePtr.h"

#include <dolbyio/comms/media_engine/media_engine.h>
#include <functional>

namespace dolbyio::comms::services
{
	class device_management;
}

namespace DolbyIO
{
	class FErrorHandler;
	class ISdkEventObserver;

	using FDvcDevice = dolbyio::comms::dvc_device;
	using EDirection = enum dolbyio::comms::dvc_device::direction;
	using Uid = std::string;
	using FHandlersMaker = std::function<FErrorHandler(int)>;

	class FDeviceManagement final
	{
	public:
		using FDvcDeviceManagement = dolbyio::comms::services::device_management;

		FDeviceManagement(FDvcDeviceManagement&, ISdkEventObserver&, FHandlersMaker);
		~FDeviceManagement();

		void SetInputDevice(int Index);
		void SetOutputDevice(int Index);
		int GetNumberOfDevices(EDirection) const;
		FDeviceNames GetDeviceNames(EDirection) const;

	private:
		const FDvcDevice& GetDeviceAt(EDirection, int Index) const;
		int GetDeviceIndex(EDirection, const FDvcDevice&) const;
		void InitializeDevices();
		void GetAllDevices();

		void UpdateCurrentInputDeviceIndex();
		void UpdateCurrentInputDeviceIndex(const FDvcDevice& Device);
		void UpdateCurrentInputDeviceIndex(int Index, const std::string& DeviceName);
		void UpdateCurrentOutputDeviceIndex();
		void UpdateCurrentOutputDeviceIndex(const FDvcDevice& Device);
		void UpdateCurrentOutputDeviceIndex(int Index, const std::string& DeviceName);

		mutable FCriticalSection AccessDevices;
		TArray<FDvcDevice> Devices;
		FDvcDeviceManagement& DeviceManagement;
		FHandlersMaker MakeHandler;
		ISdkEventObserver& Observer;
	};
}
