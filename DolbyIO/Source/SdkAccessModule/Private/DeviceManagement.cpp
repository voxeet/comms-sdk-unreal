#include "DeviceManagement.h"
#include "Common.h"
#include "Devices.h"
#include "ExceptionHandler.h"

namespace Dolby
{
	FDeviceManagement::FDeviceManagement(FDvcDeviceManagement& DeviceManagement, ISdkEventsObserver& Observer,
	                                     FExceptionHandler& ExceptionHandler)
	    : InputDevices(MakeUnique<FDevices>(FDevices::EDirection::Input, DeviceManagement, Observer, ExceptionHandler)),
	      OutputDevices(
	          MakeUnique<FDevices>(FDevices::EDirection::Output, DeviceManagement, Observer, ExceptionHandler))
	{
		InitializeDevices(DeviceManagement, ExceptionHandler);
	}

	FDeviceManagement::~FDeviceManagement() {}

	void FDeviceManagement::SetInputDevice(const int Index)
	{
		InputDevices->Set(Index);
	}

	void FDeviceManagement::SetOutputDevice(const int Index)
	{
		OutputDevices->Set(Index);
	}

	using namespace dolbyio::comms;
	namespace
	{
		bool IsInput(const dvc_device& Device)
		{
			return Device.direction() & dvc_device::input;
		}
		bool IsOutput(const dvc_device& Device)
		{
			return Device.direction() & dvc_device::output;
		}
	}

	void FDeviceManagement::InitializeDevices(FDvcDeviceManagement& DeviceManagement,
	                                          FExceptionHandler& ExceptionHandler)
	{
		GetAllDevices(DeviceManagement, ExceptionHandler);

		DeviceManagement
		    .add_event_handler(
		        [this](const device_changed& Event)
		        {
			        if (Event.no_device)
			        {
				        if (IsInput(Event.device))
				        {
					        InputDevices->OnChangedToNone();
				        }
				        if (IsOutput(Event.device))
				        {
					        OutputDevices->OnChangedToNone();
				        }
			        }
			        else
			        {
				        if (Event.utilized_direction & dvc_device::input)
				        {
					        InputDevices->OnChanged(Event.device);
				        }
				        if (Event.utilized_direction & dvc_device::output)
				        {
					        OutputDevices->OnChanged(Event.device);
				        }
			        }
		        })
		    .on_error(ExceptionHandler);

		DeviceManagement
		    .add_event_handler(
		        [this](const device_added& Event)
		        {
			        if (IsInput(Event.device))
			        {
				        InputDevices->OnAdded(Event.device);
			        }
			        if (IsOutput(Event.device))
			        {
				        OutputDevices->OnAdded(Event.device);
			        }
		        })
		    .on_error(ExceptionHandler);

		DeviceManagement
		    .add_event_handler(
		        [this](const device_removed& Event)
		        {
			        InputDevices->OnRemoved(Event.uid);
			        OutputDevices->OnRemoved(Event.uid);
		        })
		    .on_error(ExceptionHandler);
	}

	void FDeviceManagement::GetAllDevices(FDvcDeviceManagement& DeviceManagement, FExceptionHandler& ExceptionHandler)
	{
		DeviceManagement.get_audio_devices()
		    .then(
		        [this](auto&& Devices)
		        {
			        FDevices::FDvcDevices InitInputDevices;
			        FDevices::FDvcDevices InitOutputDevices;
			        FDevices::FDeviceNames InitInputDeviceNames;
			        FDevices::FDeviceNames InitOutputDeviceNames;
			        for (const auto& Device : Devices)
			        {
				        const auto DeviceName = ToFText(Device.name());
				        if (IsInput(Device))
				        {
					        InitInputDevices.Add(Device);
					        InitInputDeviceNames.Add(DeviceName);
				        }
				        if (IsOutput(Device))
				        {
					        InitOutputDevices.Add(Device);
					        InitOutputDeviceNames.Add(DeviceName);
				        }
			        }
			        InputDevices->Initialize(MoveTemp(InitInputDevices), MoveTemp(InitInputDeviceNames));
			        OutputDevices->Initialize(MoveTemp(InitOutputDevices), MoveTemp(InitOutputDeviceNames));
		        })
		    .on_error(ExceptionHandler);
	}
}
