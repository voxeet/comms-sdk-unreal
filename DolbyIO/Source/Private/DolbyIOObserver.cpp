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
#define DLB_BIND(Event) DolbyIOSubsystem->Event.AddDynamic(this, &UDolbyIOObserver::Fwd##Event);
		DLB_BIND(OnTokenNeeded);
		DLB_BIND(OnInitialized);
		DLB_BIND(OnConnected);
		DLB_BIND(OnDisconnected);
		DLB_BIND(OnParticipantAdded);
		DLB_BIND(OnParticipantUpdated);
		DLB_BIND(OnVideoTrackAdded);
		DLB_BIND(OnVideoTrackRemoved);
		DLB_BIND(OnVideoTrackEnabled);
		DLB_BIND(OnVideoTrackDisabled);
		DLB_BIND(OnVideoEnabled);
		DLB_BIND(OnVideoDisabled);
		DLB_BIND(OnScreenshareStarted);
		DLB_BIND(OnScreenshareStopped);
		DLB_BIND(OnActiveSpeakersChanged);
		DLB_BIND(OnAudioLevelsChanged);
		DLB_BIND(OnScreenshareSourcesReceived);
		DLB_BIND(OnAudioInputDevicesReceived);
		DLB_BIND(OnAudioOutputDevicesReceived)
		DLB_BIND(OnCurrentAudioInputDeviceReceived);
		DLB_BIND(OnCurrentAudioOutputDeviceReceived);
		DLB_BIND(OnVideoDevicesReceived);
		DLB_BIND(OnCurrentAudioInputDeviceChanged);
		DLB_BIND(OnCurrentAudioOutputDeviceChanged);
		FwdOnTokenNeeded();
	}
}

template <class TDelegate, class... TArgs> void UDolbyIOObserver::BroadcastEvent(TDelegate& Event, TArgs&&... Args)
{
	AsyncTask(ENamedThreads::GameThread, [=] { Event.Broadcast(Args...); });
}
