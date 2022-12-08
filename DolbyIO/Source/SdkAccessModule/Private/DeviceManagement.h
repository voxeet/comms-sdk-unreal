// Copyright 2022 Dolby Laboratories

#pragma once

#include "CommonTypes.h"

#include "Templates/UniquePtr.h"

#include <dolbyio/comms/media_engine/media_engine.h>
#include <functional>

namespace dolbyio::comms::services
{
	class device_management;
}

namespace Dolby
{
	struct FErrorHandler;
	class ISdkEventsObserver;

	using FDvcDevice = dolbyio::comms::dvc_device;
	using EDirection = enum dolbyio::comms::dvc_device::direction;
	using Uid = std::string;
	using FHandlersMaker = std::function<FErrorHandler(int)>;

	class FDeviceManagement final
	{
	public:
		using FDvcDeviceManagement = dolbyio::comms::services::device_management;

		FDeviceManagement(FDvcDeviceManagement&, ISdkEventsObserver&, FHandlersMaker);
		~FDeviceManagement();

		void SetInputDevice(const Index Index);
		void SetOutputDevice(const Index Index);
		Index GetNumberOfDevices(const EDirection direction) const;
		FDeviceNames GetDeviceNames(const EDirection direction) const;

	private:
		const FDvcDevice& GetDeviceAt(const EDirection direction, Index Index) const;
		Index GetDeviceIndex(const EDirection direction, const FDvcDevice&) const;
		void InitializeDevices();
		void GetAllDevices();

		void UpdateCurrentInputDeviceIndex();
		void UpdateCurrentInputDeviceIndex(const FDvcDevice& Device);
		void UpdateCurrentInputDeviceIndex(const Index Index, const std::string& DeviceName);
		void UpdateCurrentOutputDeviceIndex();
		void UpdateCurrentOutputDeviceIndex(const FDvcDevice& Device);
		void UpdateCurrentOutputDeviceIndex(const Index Index, const std::string& DeviceName);

		mutable FCriticalSection AccessDevices;
		TArray<FDvcDevice> Devices;
		FDvcDeviceManagement& DeviceManagement;
		FHandlersMaker MakeHandler;
		ISdkEventsObserver& Observer;
	};
}
