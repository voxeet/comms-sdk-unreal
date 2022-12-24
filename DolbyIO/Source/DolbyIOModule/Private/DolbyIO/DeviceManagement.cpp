// Copyright 2022 Dolby Laboratories

#include "DolbyIO/DeviceManagement.h"

#include "DolbyIO/ErrorHandler.h"
#include "DolbyIO/Logging.h"
#include "DolbyIO/SdkEventObserver.h"

#include <dolbyio/comms/device_management.h>

#include "Misc/ScopeLock.h"

namespace DolbyIO
{
	inline auto ToFText(const std::string& String)
	{
		return FText::FromString(UTF8_TO_TCHAR(String.c_str()));
	}

#define DLB_UE_LOG_DEVICE(Type, Event, Name) \
	UE_LOG(LogDolby, Log, TEXT(Type " device " Event ": %s"), *ToFText(Name).ToString())

	FDeviceManagement::FDeviceManagement(FDvcDeviceManagement& DeviceManagement, ISdkEventObserver& Observer,
	                                     FHandlersMaker MakeHandler)
	    : DeviceManagement(DeviceManagement), MakeHandler(MakeHandler), Observer(Observer)
	{
		InitializeDevices();
	}

	FDeviceManagement::~FDeviceManagement() {}

	void FDeviceManagement::SetInputDevice(int Index)
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

	void FDeviceManagement::UpdateCurrentInputDeviceIndex(int Index, const std::string& DeviceName)
	{
		DLB_UE_LOG_DEVICE("Input", "changed", DeviceName);
		Observer.OnCurrentInputDeviceChangedEvent(Index);
	}

	void FDeviceManagement::SetOutputDevice(int Index)
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

	void FDeviceManagement::UpdateCurrentOutputDeviceIndex(int Index, const std::string& DeviceName)
	{
		DLB_UE_LOG_DEVICE("Output", "changed", DeviceName);
		Observer.OnCurrentOutputDeviceChangedEvent(Index);
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
				        Devices.Add(Event.device); // adding devices changes lists and therefore may change
				                                   // the index of the current (input or output) device
			        }
			        if (IsInput(Event.device.direction()))
			        {
				        DLB_UE_LOG_DEVICE("Input", "added", Event.device.name());
				        UpdateCurrentInputDeviceIndex();
				        Observer.OnListOfInputDevicesChangedEvent(GetDeviceNames(EDirection::input));
			        }
			        if (IsOutput(Event.device.direction()))
			        {
				        DLB_UE_LOG_DEVICE("Output", "added", Event.device.name());
				        UpdateCurrentOutputDeviceIndex();
				        Observer.OnListOfOutputDevicesChangedEvent(GetDeviceNames(EDirection::output));
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
				        Devices.RemoveAt(Index); // removing devices changes lists and therefore may change
				                                 // the index of the current (input or output) device
			        }
			        if (IsInput(Direction))
			        {
				        UpdateCurrentInputDeviceIndex();
				        Observer.OnListOfInputDevicesChangedEvent(GetDeviceNames(EDirection::input));
			        }
			        if (IsOutput(Direction))
			        {
				        UpdateCurrentOutputDeviceIndex();
				        Observer.OnListOfOutputDevicesChangedEvent(GetDeviceNames(EDirection::output));
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
			        Observer.OnListOfInputDevicesChangedEvent(GetDeviceNames(EDirection::input));
			        Observer.OnListOfOutputDevicesChangedEvent(GetDeviceNames(EDirection::output));
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

	const FDvcDevice& FDeviceManagement::GetDeviceAt(EDirection Direction, int Index) const
	{
		for (const auto& Device : Devices) // Devices are locked at caller
		{
			if (Device.direction() & Direction)
			{
				if (Index-- == 0) // check if we are at Index'th device for given direction
				{
					return Device;
				}
			}
		}
		throw std::logic_error{std::string{"Wrong "} + (IsInput(Direction) ? "input" : "output") + " device Index"};
	}

	int FDeviceManagement::GetNumberOfDevices(EDirection Direction) const
	{
		int Count = 0;
		FScopeLock Lock{&AccessDevices};
		for (const auto& Device : Devices)
		{
			if (Device.direction() & Direction)
			{
				++Count;
			}
		}
		return Count;
	}

	int FDeviceManagement::GetDeviceIndex(const EDirection direction, const FDvcDevice& GivenDevice) const
	{
		int Index = 0;
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
