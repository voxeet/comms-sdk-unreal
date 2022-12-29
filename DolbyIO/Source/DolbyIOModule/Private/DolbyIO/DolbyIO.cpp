// Copyright 2022 Dolby Laboratories

#include "DolbyIO.h"

#include "DolbyIO/Authenticator.h"
#include "DolbyIO/SdkAccess.h"

#include "Async/Async.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

void UDolbyIO::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CppSdk = MakeShared<DolbyIO::FSdkAccess>(*this);
	Authenticator = MakeShared<DolbyIO::FAuthenticator>(*this);
	GameInstance = GetGameInstance();

	OnTokenNeeded();

	if (GameInstance)
	{
		GameInstance->GetTimerManager().SetTimer(SpatialUpdateTimerHandle, this,
		                                         &UDolbyIO::UpdateViewPointUsingFirstPlayer, 0.03, true);
	}
}

void UDolbyIO::Deinitialize()
{
	bIsAlive = false;
	CppSdk.Reset();
	Super::Deinitialize();
}

void UDolbyIO::SetToken(const FString& Token)
{
	CppSdk->SetToken(Token);
}
void UDolbyIO::SetTokenUsingKeyAndSecret(const FString& AppKey, const FString& AppSecret,
                                         int TokenExpirationTimeInSeconds)
{
	Authenticator->GetToken(AppKey, AppSecret, TokenExpirationTimeInSeconds);
}
void UDolbyIO::Connect(const FString& ConferenceName, const FString& UserName, const FString& ExternalID,
                       const FString& AvatarURL)
{
	CppSdk->Connect(ConferenceName, UserName, ExternalID, AvatarURL);
}
void UDolbyIO::Disconnect()
{
	CppSdk->Disconnect();
}
void UDolbyIO::UpdateViewPoint(const FVector& Position, const FRotator& Rotation)
{
	if (SpatialUpdateTimerHandle.IsValid())
	{
		GameInstance->GetTimerManager().ClearTimer(SpatialUpdateTimerHandle);
	}

	CppSdk->UpdateViewPoint(Position, Rotation);
}
void UDolbyIO::MuteInput()
{
	CppSdk->MuteInput();
}
void UDolbyIO::UnmuteInput()
{
	CppSdk->UnmuteInput();
}
void UDolbyIO::MuteOutput()
{
	CppSdk->MuteOutput();
}
void UDolbyIO::UnmuteOutput()
{
	CppSdk->UnmuteOutput();
}
void UDolbyIO::GetAudioLevels()
{
	CppSdk->GetAudioLevels();
}

void UDolbyIO::UpdateViewPointUsingFirstPlayer()
{
	if (GameInstance)
	{
		if (const auto World = GameInstance->GetWorld())
		{
			if (const auto FirstPlayerController = World->GetFirstPlayerController())
			{
				FVector Position;
				FRotator Rotation;
				FirstPlayerController->GetActorEyesViewPoint(Position, Rotation);
				CppSdk->UpdateViewPoint(Position, Rotation);
			}
		}
	}
}

void UDolbyIO::OnTokenNeededEvent()
{
	if (!bIsAlive)
	{
		return;
	}
	AsyncTask(ENamedThreads::GameThread, [=] { OnTokenNeeded(); });
}
void UDolbyIO::OnInitializedEvent()
{
	if (!bIsAlive)
	{
		return;
	}
	AsyncTask(ENamedThreads::GameThread, [=] { OnInitialized(); });
}
void UDolbyIO::OnConnectedEvent(const DolbyIO::FParticipantID& ParticipantID)
{
	if (!bIsAlive)
	{
		return;
	}
	AsyncTask(ENamedThreads::GameThread, [=] { OnConnected(ParticipantID); });
}
void UDolbyIO::OnDisconnectedEvent()
{
	if (!bIsAlive)
	{
		return;
	}
	AsyncTask(ENamedThreads::GameThread, [=] { OnDisconnected(); });
}
void UDolbyIO::OnParticipantAddedEvent(const FDolbyIOParticipantInfo& ParticipantInfo)
{
	if (!bIsAlive)
	{
		return;
	}
	AsyncTask(ENamedThreads::GameThread, [=] { OnParticipantAdded(ParticipantInfo); });
}
void UDolbyIO::OnParticipantLeftEvent(const FDolbyIOParticipantInfo& ParticipantInfo)
{
	if (!bIsAlive)
	{
		return;
	}
	AsyncTask(ENamedThreads::GameThread, [=] { OnParticipantLeft(ParticipantInfo); });
}
void UDolbyIO::OnActiveSpeakersChangedEvent(const DolbyIO::FParticipantIDs& Speakers)
{
	if (!bIsAlive)
	{
		return;
	}
	AsyncTask(ENamedThreads::GameThread, [=] { OnActiveSpeakersChanged(Speakers); });
}
void UDolbyIO::OnAudioLevelsChangedEvent(const DolbyIO::FAudioLevels& Levels)
{
	if (!bIsAlive)
	{
		return;
	}
	AsyncTask(ENamedThreads::GameThread, [=] { OnAudioLevelsChanged(Levels); });
}
