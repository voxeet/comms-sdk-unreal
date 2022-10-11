#pragma once

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
	struct FExceptionHandler;

	class FDevices final
	{
		using FDvcDeviceManagement = dolbyio::comms::services::device_management;
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

		FDevices(EDirection, FDvcDeviceManagement&, ISdkEventsObserver&, FExceptionHandler&);

		void Initialize(FDvcDevices&&, FDeviceNames&&);
		void Set(const Index);
		void OnAdded(const FDvcDevice&);
		void OnRemoved(const Uid&);
		void OnChanged(const FDvcDevice&);
		void OnChangedToNone();

	private:
		bool IsInput() const;
		void NotifyDeviceChanged(const FDeviceName& Name) const;
		void NotifyCurrent();

		EDirection Direction;
		FDvcDeviceManagement& DeviceManagement;
		ISdkEventsObserver& Observer;
		FExceptionHandler& ExceptionHandler;

		FCriticalSection CriticalSection;
		FDvcDevices Devices;
		FDeviceNames DeviceNames;
		FCurrentDevice CurrentDevice;
	};
}
