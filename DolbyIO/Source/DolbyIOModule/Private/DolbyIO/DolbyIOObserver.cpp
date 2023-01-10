// Copyright 2022 Dolby Laboratories

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
	if (const auto World = GetWorld())
	{
		if (const auto GameInstance = World->GetGameInstance())
		{
			if (const auto DolbyIOSubsystem = GameInstance->GetSubsystem<UDolbyIOSubsystem>())
			{
				DolbyIOSubsystem->OnTokenNeeded.AddDynamic(this, &UDolbyIOObserver::FwdOnTokenNeeded);
				DolbyIOSubsystem->OnInitialized.AddDynamic(this, &UDolbyIOObserver::FwdOnInitialized);
				DolbyIOSubsystem->OnConnected.AddDynamic(this, &UDolbyIOObserver::FwdOnConnected);
				DolbyIOSubsystem->OnDisconnected.AddDynamic(this, &UDolbyIOObserver::FwdOnDisconnected);
				DolbyIOSubsystem->OnParticipantAdded.AddDynamic(this, &UDolbyIOObserver::FwdOnParticipantAdded);
				DolbyIOSubsystem->OnParticipantLeft.AddDynamic(this, &UDolbyIOObserver::FwdOnParticipantLeft);
				DolbyIOSubsystem->OnActiveSpeakersChanged.AddDynamic(this,
				                                                     &UDolbyIOObserver::FwdOnActiveSpeakersChanged);
				DolbyIOSubsystem->OnAudioLevelsChanged.AddDynamic(this, &UDolbyIOObserver::FwdOnAudioLevelsChanged);

				FwdOnTokenNeeded();
			}
		}
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
void UDolbyIOObserver::FwdOnConnected(const FString& LocalParticipantID)
{
	BroadcastEvent(OnConnected, LocalParticipantID);
}
void UDolbyIOObserver::FwdOnDisconnected()
{
	BroadcastEvent(OnDisconnected);
}
void UDolbyIOObserver::FwdOnParticipantAdded(const FDolbyIOParticipantInfo& ParticipantInfo)
{
	BroadcastEvent(OnParticipantAdded, ParticipantInfo);
}
void UDolbyIOObserver::FwdOnParticipantLeft(const FDolbyIOParticipantInfo& ParticipantInfo)
{
	BroadcastEvent(OnParticipantLeft, ParticipantInfo);
}
void UDolbyIOObserver::FwdOnActiveSpeakersChanged(const TArray<FString>& ActiveSpeakers)
{
	BroadcastEvent(OnActiveSpeakersChanged, ActiveSpeakers);
}
void UDolbyIOObserver::FwdOnAudioLevelsChanged(const TArray<FString>& ActiveSpeakers, const TArray<float>& AudioLevels)
{
	BroadcastEvent(OnAudioLevelsChanged, ActiveSpeakers, AudioLevels);
}

template <class TDelegate, class... TArgs> void UDolbyIOObserver::BroadcastEvent(TDelegate& Event, TArgs&&... Args)
{
	AsyncTask(ENamedThreads::GameThread, [=] { Event.Broadcast(Args...); });
}
