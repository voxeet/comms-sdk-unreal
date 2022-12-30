// Copyright 2022 Dolby Laboratories

#pragma once

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
			}
		}
	}
}

void UDolbyIOObserver::FwdOnTokenNeeded()
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnTokenNeeded.Broadcast(); });
}
void UDolbyIOObserver::FwdOnInitialized()
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnInitialized.Broadcast(); });
}
void UDolbyIOObserver::FwdOnConnected(const FString& LocalParticipantID)
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnConnected.Broadcast(LocalParticipantID); });
}
void UDolbyIOObserver::FwdOnDisconnected()
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnDisconnected.Broadcast(); });
}
void UDolbyIOObserver::FwdOnParticipantAdded(const FDolbyIOParticipantInfo& ParticipantInfo)
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnParticipantAdded.Broadcast(ParticipantInfo); });
}
void UDolbyIOObserver::FwdOnParticipantLeft(const FDolbyIOParticipantInfo& ParticipantInfo)
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnParticipantLeft.Broadcast(ParticipantInfo); });
}
void UDolbyIOObserver::FwdOnActiveSpeakersChanged(const TArray<FString>& ActiveSpeakers)
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnActiveSpeakersChanged.Broadcast(ActiveSpeakers); });
}
void UDolbyIOObserver::FwdOnAudioLevelsChanged(const TArray<FString>& ActiveSpeakers, const TArray<float>& AudioLevels)
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnAudioLevelsChanged.Broadcast(ActiveSpeakers, AudioLevels); });
}
