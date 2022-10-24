#pragma once

#include "Common.h"
#include <dolbyio/comms/media_engine/media_engine.h>

namespace dolbyio::comms
{
	class dvc_device;
	namespace services
	{
		class device_management;
	}
}

namespace Dolby
{
	using FDvcDevice = dolbyio::comms::dvc_device;
	using EDirection = enum dolbyio::comms::dvc_device::direction;
	using Index = int;
	using Uid = std::string;

	struct FExceptionHandler;
	class ISdkEventsObserver;

	class FDeviceManagement final
	{
	public:
		using FDvcDeviceManagement = dolbyio::comms::services::device_management;

		FDeviceManagement(FDvcDeviceManagement&, ISdkEventsObserver&, FExceptionHandler&);
		~FDeviceManagement();

		void SetInputDevice(const Index Index);
		void SetOutputDevice(const Index Index);
		FDeviceNames GetDeviceNames(EDirection direction);

	private:
		const FDvcDevice& GetDeviceAt(EDirection direction, Index Index) const;
		Index GetDeviceIndex(EDirection direction, const FDvcDevice&) const;
		void InitializeDevices();
		void GetAllDevices();

		void UpdateCurrentInputDeviceIndex();
		void UpdateCurrentInputDeviceIndex(const FDvcDevice& Device);
		void UpdateCurrentInputDeviceIndex(Index Index, const std::string& DeviceName);
		void UpdateCurrentOutputDeviceIndex();
		void UpdateCurrentOutputDeviceIndex(const FDvcDevice& Device);
		void UpdateCurrentOutputDeviceIndex(Index Index, const std::string& DeviceName);

		FCriticalSection      AccessDevices;
		TArray<FDvcDevice>    Devices;
		FDvcDeviceManagement& DeviceManagement;
		FExceptionHandler&    ExceptionHandler;
		ISdkEventsObserver&   Observer;
	};
}
