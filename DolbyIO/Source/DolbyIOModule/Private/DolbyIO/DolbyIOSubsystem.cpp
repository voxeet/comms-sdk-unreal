// Copyright 2022 Dolby Laboratories

#include "DolbyIoSubsystem.h"

#include "DolbyIO/Authenticator.h"
#include "DolbyIO/SdkAccess.h"

#include "Async/Async.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

void UDolbyIOSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CppSdk = MakeShared<DolbyIO::FSdkAccess>(*this);
	Authenticator = MakeShared<DolbyIO::FAuthenticator>(*this);
	GameInstance = GetGameInstance();

	OnTokenNeeded();

	if (GameInstance)
	{
		GameInstance->GetTimerManager().SetTimer(SpatialUpdateTimerHandle, this,
		                                         &UDolbyIOSubsystem::UpdateViewPointUsingFirstPlayer, 0.03, true);
	}
}

void UDolbyIOSubsystem::Deinitialize()
{
	CppSdk.Reset();
	Super::Deinitialize();
}

void UDolbyIOSubsystem::SetToken(const FString& Token)
{
	CppSdk->SetToken(Token);
}
void UDolbyIOSubsystem::SetTokenUsingKeyAndSecret(const FString& AppKey, const FString& AppSecret,
                                                  int TokenExpirationTimeInSeconds)
{
	Authenticator->GetToken(AppKey, AppSecret, TokenExpirationTimeInSeconds);
}
void UDolbyIOSubsystem::Connect(const FString& ConferenceName, const FString& UserName)
{
	CppSdk->Connect(ConferenceName, UserName);
}
void UDolbyIOSubsystem::Disconnect()
{
	CppSdk->Disconnect();
}
void UDolbyIOSubsystem::UpdateViewPoint(const FVector& Position, const FRotator& Rotation)
{
	if (SpatialUpdateTimerHandle.IsValid())
	{
		GameInstance->GetTimerManager().ClearTimer(SpatialUpdateTimerHandle);
	}

	CppSdk->UpdateViewPoint(Position, Rotation);
}
void UDolbyIOSubsystem::MuteInput()
{
	CppSdk->MuteInput();
}
void UDolbyIOSubsystem::UnmuteInput()
{
	CppSdk->UnmuteInput();
}
void UDolbyIOSubsystem::MuteOutput()
{
	CppSdk->MuteOutput();
}
void UDolbyIOSubsystem::UnmuteOutput()
{
	CppSdk->UnmuteOutput();
}
void UDolbyIOSubsystem::GetAudioLevels()
{
	CppSdk->GetAudioLevels();
}
void UDolbyIOSubsystem::SetInputDevice(int Index)
{
	CppSdk->SetInputDevice(Index);
}
void UDolbyIOSubsystem::SetOutputDevice(int Index)
{
	CppSdk->SetOutputDevice(Index);
}

void UDolbyIOSubsystem::UpdateViewPointUsingFirstPlayer()
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

void UDolbyIOSubsystem::OnTokenNeededEvent()
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnTokenNeeded(); });
}
void UDolbyIOSubsystem::OnInitializedEvent()
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnInitialized(); });
}
void UDolbyIOSubsystem::OnConnectedEvent()
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnConnected(); });
}
void UDolbyIOSubsystem::OnDisconnectedEvent()
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnDisconnected(); });
}
void UDolbyIOSubsystem::OnLocalParticipantChangedEvent(const DolbyIO::FParticipant& Participant)
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnLocalParticipantChanged(Participant); });
}
void UDolbyIOSubsystem::OnListOfRemoteParticipantsChangedEvent(const DolbyIO::FParticipants& Participants)
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnListOfRemoteParticipantsChanged(Participants); });
}
void UDolbyIOSubsystem::OnListOfActiveSpeakersChangedEvent(const DolbyIO::FParticipants& Speakers)
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnListOfActiveSpeakersChanged(Speakers); });
}
void UDolbyIOSubsystem::OnListOfAudioLevelsChangedEvent(const DolbyIO::FAudioLevels& Levels)
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnListOfAudioLevelsChanged(Levels); });
}
void UDolbyIOSubsystem::OnListOfInputDevicesChangedEvent(const DolbyIO::FDeviceNames& Devices)
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnListOfInputDevicesChanged(Devices); });
}
void UDolbyIOSubsystem::OnListOfOutputDevicesChangedEvent(const DolbyIO::FDeviceNames& Devices)
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnListOfOutputDevicesChanged(Devices); });
}
void UDolbyIOSubsystem::OnCurrentInputDeviceChangedEvent(int Index)
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnCurrentInputDeviceChanged(Index); });
}
void UDolbyIOSubsystem::OnCurrentOutputDeviceChangedEvent(int Index)
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnCurrentOutputDeviceChanged(Index); });
}
