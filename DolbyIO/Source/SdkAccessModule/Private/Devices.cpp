#include "Devices.h"

#include "Common.h"

#include "Misc/ScopeLock.h"

namespace Dolby
{
	FDevices::FDevices(EDirection Direction, FDvcDeviceManagement& DeviceManagement, FSdkStatus& Status)
	    : Direction(Direction), DeviceManagement(DeviceManagement), Status(Status)
	{
	}

#define DLB_LOCK FScopeLock Lock{&CriticalSection};

	void FDevices::Initialize(FDvcDevices&& DvcDevices, FDeviceNames&& Names)
	{
		DLB_LOCK
		Devices = MoveTemp(DvcDevices);
		DeviceNames = MoveTemp(Names);
		CurrentDevice.Reset();
		IsInput() ? Status.OnNewListOfInputDevices(DeviceNames) : Status.OnNewListOfOutputDevices(DeviceNames);
		NotifyCurrent();
	}

	void FDevices::Set(const Index Index)
	{
		DLB_LOCK

		if (!Devices.IsValidIndex(Index))
		{
			throw std::logic_error{std::string{"Wrong "} + (IsInput() ? "input" : "output") + " device Index "};
		}

		return (IsInput() ? DeviceManagement.set_preferred_input_audio_device(Devices[Index])
		                  : DeviceManagement.set_preferred_output_audio_device(Devices[Index]))
		    .on_error(
		        [this](auto&& Ex)
		        {
			        NotifyCurrent();
			        HandleAsyncException(Ex, Status);
		        });
	}

	void FDevices::OnAdded(const FDvcDevice& Device)
	{
		const auto Name = ToFText(Device.name());
		DLB_LOCK
		Devices.Add(Device);
		DeviceNames.Add(Name);

		if (IsInput())
		{
			Status.OnInputDeviceAdded(Name);
			Status.OnNewListOfInputDevices(DeviceNames);
		}
		else
		{
			Status.OnOutputDeviceAdded(Name);
			Status.OnNewListOfOutputDevices(DeviceNames);
		}
	}

	void FDevices::OnRemoved(const Uid& Uid)
	{
		DLB_LOCK
		for (int i = 0; i < Devices.Num(); ++i)
		{
			if (Devices[i].uid() == Uid)
			{
				Devices.RemoveAt(i);
				const auto Name = DeviceNames[i];
				DeviceNames.RemoveAt(i);

				if (IsInput())
				{
					Status.OnInputDeviceRemoved(Name);
					Status.OnNewListOfInputDevices(DeviceNames);
				}
				else
				{
					Status.OnOutputDeviceRemoved(Name);
					Status.OnNewListOfOutputDevices(DeviceNames);
				}

				break;
			}
		}
	}

	void FDevices::OnChanged(const FDvcDevice& Device)
	{
		DLB_LOCK
		if (!CurrentDevice || !(*CurrentDevice == Device))
		{
			CurrentDevice = MakeUnique<FDvcDevice>(Device);
			const auto Name = ToFText(Device.name());
			IsInput() ? Status.OnInputDeviceChanged(Name) : Status.OnOutputDeviceChanged(Name);
		}
	}

	void FDevices::OnChangedToNone()
	{
		DLB_LOCK
		if (CurrentDevice)
		{
			CurrentDevice.Reset();
			static const auto Name = FText::FromString("None");
			IsInput() ? Status.OnInputDeviceChanged(Name) : Status.OnOutputDeviceChanged(Name);
		}
	}

	bool FDevices::IsInput() const
	{
		return Direction == EDirection::Input;
	}

	void FDevices::NotifyCurrent()
	{
		(IsInput() ? DeviceManagement.get_current_audio_input_device()
		           : DeviceManagement.get_current_audio_output_device())
		    .then([this](auto&& Device) { Device ? OnChanged(*Device) : OnChangedToNone(); })
		    .on_error(DLB_HANDLE_ASYNC_EXCEPTION);
	}
}
