#include "Devices.h"
#include "Common.h"
#include "SdkApi.h"



DECLARE_LOG_CATEGORY_EXTERN(LogDolby, Log, All);
#define DLB_UE_LOG_DEVICE(Type, Event) UE_LOG(LogDolby, Log, TEXT(Type " device " Event ": %s"), *Name.ToString())

namespace Dolby
{
	FDevices::FDevices(EDirection Direction, FDvcDeviceManagement& DeviceManagement, ISdkApi& Delegate,
	                   FExceptionHandler& ExceptionHandler)
	    : Direction(Direction), DeviceManagement(DeviceManagement), Delegate(Delegate),
	      ExceptionHandler(ExceptionHandler)
	{
	}

	void FDevices::Initialize(FDvcDevices&& DvcDevices, FDeviceNames&& Names)
	{
		FScopeLock Lock{&CriticalSection};
		Devices = MoveTemp(DvcDevices);
		DeviceNames = MoveTemp(Names);
		IsInput() ? Delegate.OnNewListOfInputDevices(DeviceNames) : Delegate.OnNewListOfOutputDevices(DeviceNames);
		NotifyCurrent();
	}

	void FDevices::Set(const Index Index)
	{
		if (!Devices.IsValidIndex(Index))
		{
			throw std::logic_error{std::string{"Wrong "} + (IsInput() ? "input" : "output") + " device Index "};
		}

		FScopeLock Lock{&CriticalSection};

		(IsInput() ? DeviceManagement.set_preferred_input_audio_device(Devices[Index])
		           : DeviceManagement.set_preferred_output_audio_device(Devices[Index]))
		    .on_error(
		        [this](auto&& Ex)
		        {
			        NotifyCurrent();
			        ExceptionHandler(std::move(Ex));
		        });
	}

	void FDevices::OnAdded(const FDvcDevice& Device)
	{
		const auto Name = ToFText(Device.name());
		FScopeLock Lock{&CriticalSection};
		Devices.Add(Device);
		DeviceNames.Add(Name);

		if (IsInput())
		{
			DLB_UE_LOG_DEVICE("Input", "added");
			Delegate.OnNewListOfInputDevices(DeviceNames);
		}
		else
		{
			DLB_UE_LOG_DEVICE("Output", "added");
			Delegate.OnNewListOfOutputDevices(DeviceNames);
		}
	}

	void FDevices::OnRemoved(const Uid& Uid)
	{
		FScopeLock Lock{&CriticalSection};
		for (int i = 0; i < Devices.Num(); ++i)
		{
			if (Devices[i].uid() == Uid)
			{
				Devices.RemoveAt(i);
				const auto Name = DeviceNames[i];
				DeviceNames.RemoveAt(i);

				if (IsInput())
				{
					DLB_UE_LOG_DEVICE("Input", "removed");
					Delegate.OnNewListOfInputDevices(DeviceNames);
				}
				else
				{
					DLB_UE_LOG_DEVICE("Output", "removed");
					Delegate.OnNewListOfOutputDevices(DeviceNames);
				}

				break;
			}
		}
	}

	void FDevices::OnChanged(const FDvcDevice& Device)
	{
		FScopeLock Lock{&CriticalSection};
		if (!CurrentDevice || !(*CurrentDevice == Device))
		{
			CurrentDevice = MakeUnique<FDvcDevice>(Device);
			NotifyDeviceChanged(ToFText(Device.name()));
		}
	}

	void FDevices::OnChangedToNone()
	{
		FScopeLock Lock{&CriticalSection};
		if (CurrentDevice)
		{
			CurrentDevice.Reset();
			static const auto Name = FText::FromString("None");
			NotifyDeviceChanged(Name);
		}
	}

	void FDevices::NotifyDeviceChanged(const FDeviceName& Name) const
	{
		if (IsInput())
		{
			DLB_UE_LOG_DEVICE("Input", "changed");
			Delegate.OnInputDeviceChanged(Name);
		}
		else
		{
			DLB_UE_LOG_DEVICE("Output", "changed");
			Delegate.OnOutputDeviceChanged(Name);
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
		    .on_error(ExceptionHandler);
	}
}
