#pragma once

#include "Internationalization/Text.h"

#include <string>

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
	class FSdkStatus;

	class FDevices final
	{
		using FDvcDeviceManagement = dolbyio::comms::services::device_management;
		using FDeviceName = FText;
		using FDvcDevice = dolbyio::comms::dvc_device;
		using FCurrentDevice = TUniquePtr<FDvcDevice>;
		using Index = int;
		using Uid = std::string;

	public:
		enum class EDirection
		{
			Input,
			Output
		};
		using FDvcDevices = TArray<FDvcDevice>;
		using FDeviceNames = TArray<FDeviceName>;

		FDevices(EDirection, FDvcDeviceManagement&, FSdkStatus&);

		void Initialize(FDvcDevices&&, FDeviceNames&&);
		void Set(const Index);
		void OnAdded(const FDvcDevice&);
		void OnRemoved(const Uid&);
		void OnChanged(const FDvcDevice&);
		void OnChangedToNone();

	private:
		bool IsInput() const;
		void NotifyCurrent();

		EDirection Direction;
		FDvcDeviceManagement& DeviceManagement;
		FSdkStatus& Status;

		FCriticalSection CriticalSection;
		FDvcDevices Devices;
		FDeviceNames DeviceNames;
		FCurrentDevice CurrentDevice;
	};
}
