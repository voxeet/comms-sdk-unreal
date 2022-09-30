#pragma once

namespace dolbyio::comms::services
{
	class device_management;
}

namespace Dolby
{
	class FDevices;
	struct FExceptionHandler;
	class ISdkApi;

	class FDeviceManagement final
	{
	public:
		using FDvcDeviceManagement = dolbyio::comms::services::device_management;

		FDeviceManagement(FDvcDeviceManagement&, ISdkApi&, FExceptionHandler&);
		~FDeviceManagement();

		void SetInputDevice(const int Index);
		void SetOutputDevice(const int Index);

	private:
		void InitializeDevices(FDvcDeviceManagement&, FExceptionHandler&);
		void GetAllDevices(FDvcDeviceManagement&, FExceptionHandler&);

		TUniquePtr<FDevices> InputDevices;
		TUniquePtr<FDevices> OutputDevices;
	};
}
