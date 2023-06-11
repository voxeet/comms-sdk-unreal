// Copyright 2023 Dolby Laboratories

#include "DolbyIOObserver.h"

#include "DolbyIOSubsystem.h"

#include "Async/Async.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

UDolbyIOObserver::UDolbyIOObserver()
{
	bWantsInitializeComponent = true;
}

void UDolbyIOObserver::InitializeComponent()
{
	auto World = GetWorld();
	if (World && World->GetGameInstance())
	{
		UDolbyIOSubsystem* DolbyIOSubsystem = World->GetGameInstance()->GetSubsystem<UDolbyIOSubsystem>();
		DolbyIOSubsystem->OnTokenNeeded.AddDynamic(this, &UDolbyIOObserver::FwdOnTokenNeeded);
		DolbyIOSubsystem->OnInitialized.AddDynamic(this, &UDolbyIOObserver::FwdOnInitialized);
		DolbyIOSubsystem->OnConnected.AddDynamic(this, &UDolbyIOObserver::FwdOnConnected);
		DolbyIOSubsystem->OnDisconnected.AddDynamic(this, &UDolbyIOObserver::FwdOnDisconnected);
		DolbyIOSubsystem->OnParticipantAdded.AddDynamic(this, &UDolbyIOObserver::FwdOnParticipantAdded);
		DolbyIOSubsystem->OnParticipantUpdated.AddDynamic(this, &UDolbyIOObserver::FwdOnParticipantUpdated);
		DolbyIOSubsystem->OnVideoTrackAdded.AddDynamic(this, &UDolbyIOObserver::FwdOnVideoTrackAdded);
		DolbyIOSubsystem->OnVideoTrackRemoved.AddDynamic(this, &UDolbyIOObserver::FwdOnVideoTrackRemoved);
		DolbyIOSubsystem->OnVideoTrackEnabled.AddDynamic(this, &UDolbyIOObserver::FwdOnVideoTrackEnabled);
		DolbyIOSubsystem->OnVideoTrackDisabled.AddDynamic(this, &UDolbyIOObserver::FwdOnVideoTrackDisabled);
		DolbyIOSubsystem->OnVideoEnabled.AddDynamic(this, &UDolbyIOObserver::FwdOnVideoEnabled);
		DolbyIOSubsystem->OnVideoDisabled.AddDynamic(this, &UDolbyIOObserver::FwdOnVideoDisabled);
		DolbyIOSubsystem->OnScreenshareStarted.AddDynamic(this, &UDolbyIOObserver::FwdOnScreenshareStarted);
		DolbyIOSubsystem->OnScreenshareStopped.AddDynamic(this, &UDolbyIOObserver::FwdOnScreenshareStopped);
		DolbyIOSubsystem->OnActiveSpeakersChanged.AddDynamic(this, &UDolbyIOObserver::FwdOnActiveSpeakersChanged);
		DolbyIOSubsystem->OnAudioLevelsChanged.AddDynamic(this, &UDolbyIOObserver::FwdOnAudioLevelsChanged);
		DolbyIOSubsystem->OnScreenshareSourcesReceived.AddDynamic(this,
		                                                          &UDolbyIOObserver::FwdOnScreenshareSourcesReceived);
		DolbyIOSubsystem->OnAudioInputDevicesReceived.AddDynamic(this,
		                                                         &UDolbyIOObserver::FwdOnAudioInputDevicesReceived);
		DolbyIOSubsystem->OnAudioOutputDevicesReceived.AddDynamic(this,
		                                                          &UDolbyIOObserver::FwdOnAudioOutputDevicesReceived);
		DolbyIOSubsystem->OnCurrentAudioInputDeviceReceived.AddDynamic(
		    this, &UDolbyIOObserver::FwdOnCurrentAudioInputDeviceReceived);
		DolbyIOSubsystem->OnCurrentAudioOutputDeviceReceived.AddDynamic(
		    this, &UDolbyIOObserver::FwdOnCurrentAudioOutputDeviceReceived);
		DolbyIOSubsystem->OnVideoDevicesReceived.AddDynamic(this, &UDolbyIOObserver::FwdOnVideoDevicesReceived);
		DolbyIOSubsystem->OnCurrentAudioInputDeviceChanged.AddDynamic(
		    this, &UDolbyIOObserver::FwdOnCurrentAudioInputDeviceChanged);
		DolbyIOSubsystem->OnCurrentAudioOutputDeviceChanged.AddDynamic(
		    this, &UDolbyIOObserver::FwdOnCurrentAudioOutputDeviceChanged);
		FwdOnTokenNeeded();
	}
}

void UDolbyIOObserver::FwdOnTokenNeeded()
{
	BroadcastEvent(OnTokenNeeded);
}
void UDolbyIOObserver::FwdOnInitialized()
{
	BroadcastEvent(OnInitialized);
}
void UDolbyIOObserver::FwdOnConnected(const FString& LocalParticipantID, const FString& ConferenceID)
{
	BroadcastEvent(OnConnected, LocalParticipantID, ConferenceID);
}
void UDolbyIOObserver::FwdOnDisconnected()
{
	BroadcastEvent(OnDisconnected);
}
void UDolbyIOObserver::FwdOnParticipantAdded(const EDolbyIOParticipantStatus Status,
                                             const FDolbyIOParticipantInfo& ParticipantInfo)
{
	BroadcastEvent(OnParticipantAdded, Status, ParticipantInfo);
}
void UDolbyIOObserver::FwdOnParticipantUpdated(const EDolbyIOParticipantStatus Status,
                                               const FDolbyIOParticipantInfo& ParticipantInfo)
{
	BroadcastEvent(OnParticipantUpdated, Status, ParticipantInfo);
}
void UDolbyIOObserver::FwdOnVideoTrackAdded(const FDolbyIOVideoTrack& VideoTrack)
{
	BroadcastEvent(OnVideoTrackAdded, VideoTrack);
}
void UDolbyIOObserver::FwdOnVideoTrackRemoved(const FDolbyIOVideoTrack& VideoTrack)
{
	BroadcastEvent(OnVideoTrackRemoved, VideoTrack);
}
void UDolbyIOObserver::FwdOnVideoTrackEnabled(const FDolbyIOVideoTrack& VideoTrack)
{
	BroadcastEvent(OnVideoTrackEnabled, VideoTrack);
}
void UDolbyIOObserver::FwdOnVideoTrackDisabled(const FDolbyIOVideoTrack& VideoTrack)
{
	BroadcastEvent(OnVideoTrackDisabled, VideoTrack);
}
void UDolbyIOObserver::FwdOnVideoEnabled(const FString& VideoTrackID)
{
	BroadcastEvent(OnVideoEnabled, VideoTrackID);
}
void UDolbyIOObserver::FwdOnVideoDisabled(const FString& VideoTrackID)
{
	BroadcastEvent(OnVideoDisabled, VideoTrackID);
}
void UDolbyIOObserver::FwdOnScreenshareStarted(const FString& VideoTrackID)
{
	BroadcastEvent(OnScreenshareStarted, VideoTrackID);
}
void UDolbyIOObserver::FwdOnScreenshareStopped(const FString& VideoTrackID)
{
	BroadcastEvent(OnScreenshareStopped, VideoTrackID);
}
void UDolbyIOObserver::FwdOnActiveSpeakersChanged(const TArray<FString>& ActiveSpeakers)
{
	BroadcastEvent(OnActiveSpeakersChanged, ActiveSpeakers);
}
void UDolbyIOObserver::FwdOnAudioLevelsChanged(const TArray<FString>& ActiveSpeakers, const TArray<float>& AudioLevels)
{
	BroadcastEvent(OnAudioLevelsChanged, ActiveSpeakers, AudioLevels);
}
void UDolbyIOObserver::FwdOnScreenshareSourcesReceived(const TArray<FDolbyIOScreenshareSource>& Sources)
{
	BroadcastEvent(OnScreenshareSourcesReceived, Sources);
}
void UDolbyIOObserver::FwdOnAudioInputDevicesReceived(const TArray<FDolbyIOAudioDevice>& Devices)
{
	BroadcastEvent(OnAudioInputDevicesReceived, Devices);
}
void UDolbyIOObserver::FwdOnAudioOutputDevicesReceived(const TArray<FDolbyIOAudioDevice>& Devices)
{
	BroadcastEvent(OnAudioOutputDevicesReceived, Devices);
}
void UDolbyIOObserver::FwdOnCurrentAudioInputDeviceReceived(bool IsNone, const FDolbyIOAudioDevice& OptionalDevice)
{
	BroadcastEvent(OnCurrentAudioInputDeviceReceived, IsNone, OptionalDevice);
}
void UDolbyIOObserver::FwdOnCurrentAudioOutputDeviceReceived(bool IsNone, const FDolbyIOAudioDevice& OptionalDevice)
{
	BroadcastEvent(OnCurrentAudioOutputDeviceReceived, IsNone, OptionalDevice);
}
void UDolbyIOObserver::FwdOnVideoDevicesReceived(const TArray<FDolbyIOVideoDevice>& Devices)
{
	BroadcastEvent(OnVideoDevicesReceived, Devices);
}
void UDolbyIOObserver::FwdOnCurrentAudioInputDeviceChanged(bool IsNone, const FDolbyIOAudioDevice& OptionalDevice)
{
	BroadcastEvent(OnCurrentAudioInputDeviceChanged, IsNone, OptionalDevice);
}
void UDolbyIOObserver::FwdOnCurrentAudioOutputDeviceChanged(bool IsNone, const FDolbyIOAudioDevice& OptionalDevice)
{
	BroadcastEvent(OnCurrentAudioOutputDeviceChanged, IsNone, OptionalDevice);
}

template <class TDelegate, class... TArgs> void UDolbyIOObserver::BroadcastEvent(TDelegate& Event, TArgs&&... Args)
{
	AsyncTask(ENamedThreads::GameThread, [=] { Event.Broadcast(Args...); });
}
