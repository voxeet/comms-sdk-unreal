// Copyright 2022 Dolby Laboratories

#include "DeviceManagement.h"

#include "Common.h"
#include "ErrorHandler.h"
#include "SdkEventsObserver.h"

#include "Misc/ScopeLock.h"

namespace Dolby
{
	inline auto ToFText(const std::string& String)
	{
		return FText::FromString(UTF8_TO_TCHAR(String.c_str()));
	}

#define DLB_UE_LOG_DEVICE(Type, Event, Name) \
	UE_LOG(LogDolby, Log, TEXT(Type " device " Event ": %s"), *ToFText(Name).ToString())

	FDeviceManagement::FDeviceManagement(FDvcDeviceManagement& DeviceManagement, ISdkEventsObserver& Observer,
	                                     FHandlersMaker MakeHandler)
	    : DeviceManagement(DeviceManagement), MakeHandler(MakeHandler), Observer(Observer)
	{
		InitializeDevices();
	}

	FDeviceManagement::~FDeviceManagement() {}

	void FDeviceManagement::SetInputDevice(const Index Index)
	try
	{
		FScopeLock Lock{&AccessDevices};
		const FDvcDevice& Device = GetDeviceAt(EDirection::input, Index);
		DLB_UE_LOG_DEVICE("Input", "change requested", Device.name());
		DeviceManagement.set_preferred_input_audio_device(Device).on_error(
		    [this](auto&& Ex)
		    {
			    UpdateCurrentInputDeviceIndex();
			    MakeHandler(__LINE__)(MoveTemp(Ex));
		    });
	}
	catch (...)
	{
		MakeHandler(__LINE__).RethrowAndUpdateStatus();
	}

	void FDeviceManagement::UpdateCurrentInputDeviceIndex()
	{
		DeviceManagement.get_current_audio_input_device()
		    .then([this](auto&& Device)
		          { Device ? UpdateCurrentInputDeviceIndex(*Device) : UpdateCurrentInputDeviceIndex(-1, "<None>"); })
		    .on_error(MakeHandler(__LINE__));
	}

	void FDeviceManagement::UpdateCurrentInputDeviceIndex(const FDvcDevice& Device)
	{
		UpdateCurrentInputDeviceIndex(GetDeviceIndex(EDirection::input, Device), Device.name());
	}

	void FDeviceManagement::UpdateCurrentInputDeviceIndex(const Index Index, const std::string& DeviceName)
	{
		DLB_UE_LOG_DEVICE("Input", "changed", DeviceName);
		Observer.OnInputDeviceChanged(Index);
	}

	void FDeviceManagement::SetOutputDevice(const Index Index)
	{
		try
		{
			FScopeLock Lock{&AccessDevices};
			const FDvcDevice& Device = GetDeviceAt(EDirection::output, Index);
			DLB_UE_LOG_DEVICE("Output", "change requested", Device.name());
			DeviceManagement.set_preferred_output_audio_device(Device).on_error(
			    [this](auto&& Ex)
			    {
				    UpdateCurrentOutputDeviceIndex();
				    MakeHandler(__LINE__)(MoveTemp(Ex));
			    });
		}
		catch (...)
		{
			MakeHandler(__LINE__).RethrowAndUpdateStatus();
		}
	}

	void FDeviceManagement::UpdateCurrentOutputDeviceIndex()
	{
		DeviceManagement.get_current_audio_output_device()
		    .then([this](auto&& Device)
		          { Device ? UpdateCurrentOutputDeviceIndex(*Device) : UpdateCurrentOutputDeviceIndex(-1, "<None>"); })
		    .on_error(MakeHandler(__LINE__));
	}

	void FDeviceManagement::UpdateCurrentOutputDeviceIndex(const FDvcDevice& Device)
	{
		UpdateCurrentOutputDeviceIndex(GetDeviceIndex(EDirection::output, Device), Device.name());
	}

	void FDeviceManagement::UpdateCurrentOutputDeviceIndex(const Index Index, const std::string& DeviceName)
	{
		DLB_UE_LOG_DEVICE("Output", "changed", DeviceName);
		Observer.OnOutputDeviceChanged(Index);
	}

	using namespace dolbyio::comms;
	namespace
	{
		bool IsInput(const EDirection Direction)
		{
			return Direction & EDirection::input;
		}
		bool IsOutput(const EDirection Direction)
		{
			return Direction & EDirection::output;
		}
	}

	void FDeviceManagement::InitializeDevices()
	{
		DeviceManagement
		    .add_event_handler(
		        [this](const audio_device_changed& Event)
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
		    .on_error(MakeHandler(__LINE__));

		DeviceManagement
		    .add_event_handler(
		        [this](const audio_device_added& Event)
		        {
			        {
				        FScopeLock Lock{&AccessDevices};
				        Devices.Add(Event.device);
			        }
			        if (IsInput(Event.device.direction()))
			        {
				        DLB_UE_LOG_DEVICE("Input", "added", Event.device.name());
				        UpdateCurrentInputDeviceIndex();
				        Observer.OnListOfInputDevicesChanged();
			        }
			        if (IsOutput(Event.device.direction()))
			        {
				        DLB_UE_LOG_DEVICE("Output", "added", Event.device.name());
				        UpdateCurrentOutputDeviceIndex();
				        Observer.OnListOfOutputDevicesChanged();
			        }
		        })
		    .on_error(MakeHandler(__LINE__));

		DeviceManagement
		    .add_event_handler(
		        [this](const audio_device_removed& Event)
		        {
			        EDirection Direction{EDirection::none};
			        {
				        FScopeLock Lock{&AccessDevices};
				        const auto Index = Devices.IndexOfByPredicate([UidToRemove = Event.uid](const auto& Uid)
				                                                      { return Uid == UidToRemove; });
				        DLB_UE_LOG_DEVICE("", "removed", Devices[Index].name());
				        Direction = Devices[Index].direction();
				        Devices.RemoveAt(Index);
			        }
			        if (IsInput(Direction))
			        {
				        UpdateCurrentInputDeviceIndex();
				        Observer.OnListOfInputDevicesChanged();
			        }
			        if (IsOutput(Direction))
			        {
				        UpdateCurrentOutputDeviceIndex();
				        Observer.OnListOfOutputDevicesChanged();
			        }
		        })
		    .on_error(MakeHandler(__LINE__));

		GetAllDevices();
	}

	void FDeviceManagement::GetAllDevices()
	{
		DeviceManagement.get_audio_devices()
		    .then(
		        [this](auto&& DvcDevices)
		        {
			        {
				        FScopeLock Lock{&AccessDevices};
				        Devices.Append(DvcDevices.data(), DvcDevices.size());
				        DLB_UE_LOG("%d device(s) added (%d in total)", DvcDevices.size(), Devices.Num());
			        }
			        Observer.OnListOfInputDevicesChanged();
			        Observer.OnListOfOutputDevicesChanged();
			        if (GetNumberOfDevices(EDirection::input) > 0)
			        {
				        SetInputDevice(0);
			        }
			        if (GetNumberOfDevices(EDirection::output) > 0)
			        {
				        SetOutputDevice(0);
			        }
		        })
		    .on_error(MakeHandler(__LINE__));
	}

	FDeviceNames FDeviceManagement::GetDeviceNames(const EDirection direction) const
	{
		FDeviceNames Names;
		FScopeLock Lock{&AccessDevices};
		for (const auto& Device : Devices)
		{
			if (Device.direction() & direction)
			{
				Names.Add(ToFText(Device.name()));
			}
		}
		return Names;
	}

	const FDvcDevice& FDeviceManagement::GetDeviceAt(const EDirection direction, Index Index) const
	{
		for (const auto& Device : Devices) // Devices are locked at caller
		{
			if (Device.direction() & direction)
			{
				if (Index-- == 0) // check if we are at Index'th device for given direction
				{
					return Device;
				}
			}
		}
		throw std::logic_error{std::string{"Wrong "} + (IsInput(direction) ? "input" : "output") + " device Index"};
	}

	Index FDeviceManagement::GetNumberOfDevices(const EDirection direction) const
	{
		Index Count{0};
		FScopeLock Lock{&AccessDevices};
		for (const auto& Device : Devices)
		{
			if (Device.direction() & direction)
			{
				++Count;
			}
		}
		return Count;
	}

	Index FDeviceManagement::GetDeviceIndex(const EDirection direction, const FDvcDevice& GivenDevice) const
	{
		Index Index = 0;
		FScopeLock Lock{&AccessDevices};
		for (const auto& Device : Devices)
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
