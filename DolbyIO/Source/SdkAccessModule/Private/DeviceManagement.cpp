#include "DeviceManagement.h"
#include "Common.h"
#include "ExceptionHandler.h"
#include "SdkEventsObserver.h"

#define DLB_UE_LOG_DEVICE(Type, Event, Name) \
	UE_LOG(LogDolby, Log, TEXT(Type " device " Event ": %s"), *ToFText(Name.c_str()).ToString())

namespace Dolby
{
	FDeviceManagement::FDeviceManagement(FDvcDeviceManagement& DeviceManagement, ISdkEventsObserver& Observer,
	                                     FExceptionHandler& ExceptionHandler)
	    : DeviceManagement(DeviceManagement), ExceptionHandler(ExceptionHandler), Observer(Observer)
	{
		InitializeDevices();
	}

	FDeviceManagement::~FDeviceManagement() {}

	void FDeviceManagement::SetInputDevice(const Index Index)
	try
	{
		FScopeLock Lock{&AccessDevices};
		auto Device = GetDeviceAt(EDirection::input, Index);
		DLB_UE_LOG_DEVICE("Input", "change requested", Device.name());
		DeviceManagement.set_preferred_input_audio_device(Device).on_error(
			[this](auto&& Ex)
			{
				UpdateCurrentInputDeviceIndex();
				ExceptionHandler(std::move(Ex));
			});
	}
	catch (...)
	{
		ExceptionHandler.RethrowAndUpdateStatus();
	}

	void FDeviceManagement::UpdateCurrentInputDeviceIndex()
	{
		DeviceManagement.get_current_audio_input_device()
		    .then([this](auto&& Device)
		          { Device ? UpdateCurrentInputDeviceIndex(*Device) : UpdateCurrentInputDeviceIndex(-1, "<None>"); })
		    .on_error(ExceptionHandler);
	}

	void FDeviceManagement::UpdateCurrentInputDeviceIndex(const FDvcDevice& Device)
	{
		UpdateCurrentInputDeviceIndex(GetDeviceIndex(EDirection::input, Device), Device.name());
	}

	void FDeviceManagement::UpdateCurrentInputDeviceIndex(Index Index, const std::string& DeviceName)
	{
		DLB_UE_LOG_DEVICE("Input", "changed", DeviceName);
		Observer.OnInputDeviceChanged(Index);
	}

	void FDeviceManagement::SetOutputDevice(const Index Index)
	{
		try
		{
			FScopeLock Lock{&AccessDevices};
			auto Device = GetDeviceAt(EDirection::output, Index);
			DLB_UE_LOG_DEVICE("Output", "change requested", Device.name());
			DeviceManagement.set_preferred_output_audio_device(Device).on_error(
			    [this](auto&& Ex)
			    {
				    UpdateCurrentOutputDeviceIndex();
				    ExceptionHandler(std::move(Ex));
			    });
		}
		catch (...)
		{
			ExceptionHandler.RethrowAndUpdateStatus();
		}
	}

	void FDeviceManagement::UpdateCurrentOutputDeviceIndex()
	{
		DeviceManagement.get_current_audio_output_device()
		    .then([this](auto&& Device)
		          { Device ? UpdateCurrentOutputDeviceIndex(*Device) : UpdateCurrentOutputDeviceIndex(-1, "<None>"); })
		    .on_error(ExceptionHandler);
	}

	void FDeviceManagement::UpdateCurrentOutputDeviceIndex(const FDvcDevice& Device)
	{
		UpdateCurrentOutputDeviceIndex(GetDeviceIndex(EDirection::output, Device), Device.name());
	}

	void FDeviceManagement::UpdateCurrentOutputDeviceIndex(Index Index, const std::string& DeviceName)
	{
		DLB_UE_LOG_DEVICE("Output", "changed", DeviceName);
		Observer.OnOutputDeviceChanged(Index);
	}


	using namespace dolbyio::comms;
	namespace
	{
		bool IsInput(EDirection Direction)
		{
			return Direction & FDvcDevice::input;
		}
		bool IsOutput(EDirection Direction)
		{
			return Direction & FDvcDevice::output;
		}
	}

	void FDeviceManagement::InitializeDevices()
	{
		GetAllDevices();

		DeviceManagement
		    .add_event_handler(
		        [this](const device_changed& Event)
		        {
				    if (IsInput(Event.utilized_direction))
				    {
				        if (Event.no_device)
				        {
					        UpdateCurrentInputDeviceIndex(-1, "<None>");
				        }
						else
						{
							UpdateCurrentInputDeviceIndex(Event.device);
						}
				    }
				    if (IsOutput(Event.utilized_direction))
				    {
				        if (Event.no_device)
				        {
					        UpdateCurrentOutputDeviceIndex(-1, "<None>");
				        }
				        else
				        {
					        UpdateCurrentOutputDeviceIndex(Event.device);
				        }
				    }
		        })
		    .on_error(ExceptionHandler);

		DeviceManagement
		    .add_event_handler(
		        [this](const device_added& Event)
		        {
			        {
				        FScopeLock Lock{&AccessDevices};
				        Devices.Add(Event.device);
			        }
			        if (IsInput(Event.device.direction()))
			        {
				        DLB_UE_LOG_DEVICE("Input", "added", Event.device.name());
						Observer.OnListOfInputDevicesChanged();
			        }
			        if (IsOutput(Event.device.direction()))
			        {
				        DLB_UE_LOG_DEVICE("Output", "added", Event.device.name());
				        Observer.OnListOfOutputDevicesChanged();
			        }
		        })
		    .on_error(ExceptionHandler);

		DeviceManagement
		    .add_event_handler(
		        [this](const device_removed& Event)
		        {
			        EDirection Direction{EDirection::none};
			        {
				        FScopeLock Lock{&AccessDevices};
				        // simple: Devices.RemoveAll([UidToRemove = Event.uid](auto const& Uid) { return Uid == UidToRemove; });
				        auto Index = Devices.IndexOfByPredicate([UidToRemove = Event.uid](auto const& Uid)
				                                                { return Uid == UidToRemove; });
				        DLB_UE_LOG_DEVICE("In/Output", "removed", Devices[Index].name());
				        Direction = Devices[Index].direction();
				        Devices.RemoveAt(Index);
			        }
			        if (IsInput(Direction))
			        {
				        Observer.OnListOfInputDevicesChanged();
						//UpdateCurrentInputDeviceIndex(); // or rely on device_changed event?
			        }
			        if (IsOutput(Direction))
			        {
				        Observer.OnListOfOutputDevicesChanged();
				        //UpdateCurrentOutputDeviceIndex(); // or rely on device_changed event?
			        }
		        })
		    .on_error(ExceptionHandler);
	}

	void FDeviceManagement::GetAllDevices()
	{
		DeviceManagement.get_audio_devices()
		    .then(
		        [this](auto&& DvcDevices)
		        {
			        {
				        FScopeLock Lock{&AccessDevices};
				        // Devices.Append(DvcDevices.data(), DvcDevices.size()); TODO move/swap?
				        //
				        for (const auto& Device : DvcDevices)
				        {
					        auto i = Devices.Add(Device);
					        DLB_UE_LOG_DEVICE("In/Output", "init", Device.name());
				        }
			        }
			        UpdateCurrentInputDeviceIndex();
			        UpdateCurrentOutputDeviceIndex();
			        Observer.OnListOfInputDevicesChanged();
			        Observer.OnListOfOutputDevicesChanged();
		        })
		    .on_error(ExceptionHandler);
	}

	FDeviceNames FDeviceManagement::GetDeviceNames(EDirection direction)
	{
		FDeviceNames names;
		FScopeLock Lock{&AccessDevices};
		for (auto const& Device : Devices)
		{
			if (Device.direction() & direction)
			{
				DLB_UE_LOG_DEVICE("In/Output", "GetDeviceNames", Device.name());
				names.Add(ToFText(Device.name()));
			}
		}
		return names;
	}

	const FDvcDevice& FDeviceManagement::GetDeviceAt(EDirection direction, Index Index) const
	{
		for (auto const& Device : Devices)
		{
			if (Device.direction() & direction)
			{
				if (Index-- == 0)
					return Device;
			}
		}
		throw std::logic_error{std::string{"Wrong "} + (IsInput(direction) ? "input" : "output") + " device Index "};
	}

	Index FDeviceManagement::GetDeviceIndex(EDirection direction, const FDvcDevice& GivenDevice) const
	{
		Index Index = 0;
		for (auto const& Device : Devices)
		{
			if (Device == GivenDevice)
			{
				return Index;
			}
			if (Device.direction() & direction)
			{
				++Index;
			}
		}
		return -1;
	}
}
