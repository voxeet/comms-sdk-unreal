#pragma once

#include "Templates/UniquePtr.h"

namespace dolbyio::comms::services
{
	class device_management;
}

namespace Dolby
{
	class FSdkStatus;
	class FDevices;

	class FDeviceManagement final
	{
	public:
		using FDvcDeviceManagement = dolbyio::comms::services::device_management;

		FDeviceManagement(FDvcDeviceManagement&, FSdkStatus&);
		~FDeviceManagement();

		void SetInputDevice(const int Index);
		void SetOutputDevice(const int Index);

	private:
		void InitializeDevices(FDvcDeviceManagement&);
		void GetAllDevices(FDvcDeviceManagement&);

		FSdkStatus& Status;
		TUniquePtr<FDevices> InputDevices;
		TUniquePtr<FDevices> OutputDevices;
	};
}
