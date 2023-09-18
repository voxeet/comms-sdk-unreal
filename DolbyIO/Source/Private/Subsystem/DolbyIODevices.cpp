// Copyright 2023 Dolby Laboratories

#include "DolbyIODevices.h"

#include "DolbyIO.h"
#include "Utils/DolbyIOBroadcastEvent.h"
#include "Utils/DolbyIOConversions.h"
#include "Utils/DolbyIOErrorHandler.h"
#include "Utils/DolbyIOLogging.h"

using namespace dolbyio::comms;
using namespace DolbyIO;

#define DLB_DEVICES(OnError)                                 \
	if (!Devices)                                            \
	{                                                        \
		DLB_WARNING(OnError, #OnError " - not initialized"); \
		return;                                              \
	}                                                        \
	Devices

void UDolbyIOSubsystem::GetAudioInputDevices()
{
	DLB_DEVICES(OnGetAudioInputDevicesError)->GetAudioInputDevices();
}
void UDolbyIOSubsystem::GetAudioOutputDevices()
{
	DLB_DEVICES(OnGetAudioOutputDevicesError)->GetAudioOutputDevices();
}
void UDolbyIOSubsystem::GetCurrentAudioInputDevice()
{
	DLB_DEVICES(OnGetCurrentAudioInputDeviceError)->GetCurrentAudioInputDevice();
}
void UDolbyIOSubsystem::GetCurrentAudioOutputDevice()
{
	DLB_DEVICES(OnGetCurrentAudioOutputDeviceError)->GetCurrentAudioOutputDevice();
}
void UDolbyIOSubsystem::SetAudioInputDevice(const FString& NativeID)
{
	DLB_DEVICES(OnSetAudioInputDeviceError)->SetAudioInputDevice(NativeID);
}
void UDolbyIOSubsystem::SetAudioOutputDevice(const FString& NativeID)
{
	DLB_DEVICES(OnSetAudioOutputDeviceError)->SetAudioOutputDevice(NativeID);
}
void UDolbyIOSubsystem::GetVideoDevices()
{
	DLB_DEVICES(OnGetVideoDevicesError)->GetVideoDevices();
}
void UDolbyIOSubsystem::GetCurrentVideoDevice()
{
	DLB_DEVICES(OnGetCurrentVideoDeviceError)->GetCurrentVideoDevice();
}

namespace DolbyIO
{
	constexpr bool bIsDeviceNone = true;

	FDevices::FDevices(UDolbyIOSubsystem& Subsystem, FDeviceManagement& DeviceManagement)
	    : Subsystem(Subsystem), DeviceManagement(DeviceManagement)
	{
	}

	void FDevices::GetAudioInputDevices()
	{
		DLB_UE_LOG("Getting audio input devices");
		DeviceManagement.get_audio_devices()
		    .then(
		        [this](const std::vector<audio_device>& DvcDevices)
		        {
			        TArray<FDolbyIOAudioDevice> Devices;
			        for (const audio_device& Device : DvcDevices)
			        {
				        if (Device.direction() & audio_device::direction::input)
				        {
					        DLB_UE_LOG("Got audio input device: %s", *ToString(Device));
					        Devices.Add(ToFDolbyIOAudioDevice(Device));
				        }
			        }
			        BroadcastEvent(Subsystem.OnAudioInputDevicesReceived, Devices);
		        })
		    .on_error(DLB_ERROR_HANDLER(Subsystem.OnGetAudioInputDevicesError));
	}

	void FDevices::GetAudioOutputDevices()
	{
		DLB_UE_LOG("Getting audio output devices");
		DeviceManagement.get_audio_devices()
		    .then(
		        [this](const std::vector<audio_device>& DvcDevices)
		        {
			        TArray<FDolbyIOAudioDevice> Devices;
			        for (const audio_device& Device : DvcDevices)
			        {
				        if (Device.direction() & audio_device::direction::output)
				        {
					        DLB_UE_LOG("Got audio output device: %s", *ToString(Device));
					        Devices.Add(ToFDolbyIOAudioDevice(Device));
				        }
			        }
			        BroadcastEvent(Subsystem.OnAudioOutputDevicesReceived, Devices);
		        })
		    .on_error(DLB_ERROR_HANDLER(Subsystem.OnGetAudioOutputDevicesError));
	}

	void FDevices::GetCurrentAudioInputDevice()
	{
		DLB_UE_LOG("Getting current audio input device");
		DeviceManagement.get_current_audio_input_device()
		    .then(
		        [this](std::optional<audio_device> Device)
		        {
			        if (!Device)
			        {
				        DLB_UE_LOG("Got current audio input device - none");
				        BroadcastEvent(Subsystem.OnCurrentAudioInputDeviceReceivedNone);
				        return;
			        }
			        DLB_UE_LOG("Got current audio input device - %s", *ToString(*Device));
			        BroadcastEvent(Subsystem.OnCurrentAudioInputDeviceReceived, ToFDolbyIOAudioDevice(*Device));
		        })
		    .on_error(DLB_ERROR_HANDLER(Subsystem.OnGetCurrentAudioInputDeviceError));
	}

	void FDevices::GetCurrentAudioOutputDevice()
	{
		DLB_UE_LOG("Getting current audio output device");
		DeviceManagement.get_current_audio_output_device()
		    .then(
		        [this](std::optional<audio_device> Device)
		        {
			        if (!Device)
			        {
				        DLB_UE_LOG("Got current audio output device - none");
				        BroadcastEvent(Subsystem.OnCurrentAudioOutputDeviceReceivedNone);
				        return;
			        }
			        DLB_UE_LOG("Got current audio output device - %s", *ToString(*Device));
			        BroadcastEvent(Subsystem.OnCurrentAudioOutputDeviceReceived, ToFDolbyIOAudioDevice(*Device));
		        })
		    .on_error(DLB_ERROR_HANDLER(Subsystem.OnGetCurrentAudioOutputDeviceError));
	}

	void FDevices::SetAudioInputDevice(const FString& NativeID)
	{
		DLB_UE_LOG("Setting audio input device with native ID %s", *NativeID);
		DeviceManagement.get_audio_devices()
		    .then(
		        [this, SdkNativeID = ToSdkNativeDeviceID(NativeID)](const std::vector<audio_device>& DvcDevices)
		        {
			        for (const audio_device& Device : DvcDevices)
				        if (Device.direction() & audio_device::direction::input && Device.native_id() == SdkNativeID)
				        {
					        DLB_UE_LOG("Setting audio input device to %s", *ToString(Device));
					        DeviceManagement.set_preferred_input_audio_device(Device).on_error(
					            DLB_ERROR_HANDLER(Subsystem.OnSetAudioInputDeviceError));
					        return;
				        }
		        })
		    .on_error(DLB_ERROR_HANDLER(Subsystem.OnSetAudioInputDeviceError));
	}

	void FDevices::SetAudioOutputDevice(const FString& NativeID)
	{
		DLB_UE_LOG("Setting audio output device with native ID %s", *NativeID);
		DeviceManagement.get_audio_devices()
		    .then(
		        [this, SdkNativeID = ToSdkNativeDeviceID(NativeID)](const std::vector<audio_device>& DvcDevices)
		        {
			        for (const audio_device& Device : DvcDevices)
				        if (Device.direction() & audio_device::direction::output && Device.native_id() == SdkNativeID)
				        {
					        DLB_UE_LOG("Setting audio output device to %s", *ToString(Device));
					        DeviceManagement.set_preferred_output_audio_device(Device).on_error(
					            DLB_ERROR_HANDLER(Subsystem.OnSetAudioOutputDeviceError));
					        return;
				        }
		        })
		    .on_error(DLB_ERROR_HANDLER(Subsystem.OnSetAudioOutputDeviceError));
	}

	void FDevices::GetVideoDevices()
	{
		DLB_UE_LOG("Getting video devices");
		DeviceManagement.get_video_devices()
		    .then(
		        [this](const std::vector<camera_device>& DvcDevices)
		        {
			        TArray<FDolbyIOVideoDevice> Devices;
			        Devices.Reserve(DvcDevices.size());
			        for (const camera_device& Device : DvcDevices)
			        {
				        DLB_UE_LOG("Got video device - display_name: %s unique_id: %s", *ToFString(Device.display_name),
				                   *ToFString(Device.unique_id));
				        Devices.Add(ToFDolbyIOVideoDevice(Device));
			        }
			        BroadcastEvent(Subsystem.OnVideoDevicesReceived, Devices);
		        })
		    .on_error(DLB_ERROR_HANDLER(Subsystem.OnGetVideoDevicesError));
	}

	void FDevices::GetCurrentVideoDevice()
	{
		DLB_UE_LOG("Getting current video device");
		DeviceManagement.get_current_video_device()
		    .then(
		        [this](std::optional<camera_device> Device)
		        {
			        if (!Device)
			        {
				        DLB_UE_LOG("Got current video device - none");
				        BroadcastEvent(Subsystem.OnCurrentVideoDeviceReceivedNone);
				        return;
			        }
			        DLB_UE_LOG("Got current video device - %s", *ToString(*Device));
			        BroadcastEvent(Subsystem.OnCurrentVideoDeviceReceived, ToFDolbyIOVideoDevice(*Device));
		        })
		    .on_error(DLB_ERROR_HANDLER(Subsystem.OnGetCurrentVideoDeviceError));
	}
}

void UDolbyIOSubsystem::Handle(const audio_device_changed& Event)
{
	using namespace DolbyIO;

	if (!Event.device)
	{
		DLB_UE_LOG("Audio device changed for direction: %s to no device", *ToString(Event.utilized_direction));
		if (Event.utilized_direction == audio_device::direction::input)
			BroadcastEvent(OnCurrentAudioInputDeviceChangedToNone);
		else
			BroadcastEvent(OnCurrentAudioOutputDeviceChangedToNone);
		return;
	}
	Sdk->device_management()
	    .get_audio_devices()
	    .then(
	        [this, Event](const std::vector<audio_device>& DvcDevices)
	        {
		        for (const audio_device& Device : DvcDevices)
			        if (*Event.device == Device.get_identity())
			        {
				        DLB_UE_LOG("Audio device changed for direction: %s to device - %s",
				                   *ToString(Event.utilized_direction), *ToString(Device));
				        if (Event.utilized_direction == audio_device::direction::input)
					        BroadcastEvent(OnCurrentAudioInputDeviceChanged, ToFDolbyIOAudioDevice(Device));
				        else
					        BroadcastEvent(OnCurrentAudioOutputDeviceChanged, ToFDolbyIOAudioDevice(Device));
				        return;
			        }
	        })
	    .on_error(DLB_ERROR_HANDLER_NO_DELEGATE);
}
