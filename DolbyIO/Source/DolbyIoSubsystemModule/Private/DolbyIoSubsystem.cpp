// Copyright 2022 Dolby Laboratories

#include "DolbyIoSubsystem.h"

#include "DolbyIoAuthentication.h"
#include "DolbyIoSdkAccess.h"

#include "Async/Async.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Modules/ModuleManager.h"
#include "TimerManager.h"

IMPLEMENT_MODULE(FDefaultModuleImpl, DolbyIoSubsystemModule)

void UDolbyIoSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CppSdk = MakeShared<Dolby::FSdkAccess>(*this);
	Authenticator = MakeShared<Dolby::FAuthenticator>(*this);
	GameInstance = GetGameInstance();

	OnTokenNeeded();

	if (GameInstance)
	{
		GameInstance->GetTimerManager().SetTimer(SpatialUpdateTimerHandle, this,
		                                         &UDolbyIoSubsystem::UpdateViewPointUsingFirstPlayer, 0.03, true);
	}
}

void UDolbyIoSubsystem::Deinitialize()
{
	CppSdk.Reset();
	Super::Deinitialize();
}

void UDolbyIoSubsystem::SetToken(const FString& Token)
{
	CppSdk->SetToken(Token);
}
void UDolbyIoSubsystem::SetTokenUsingKeyAndSecret(const FString& AppKey, const FString& AppSecret,
                                                  int TokenExpirationTimeInSeconds)
{
	Authenticator->GetToken(AppKey, AppSecret, TokenExpirationTimeInSeconds);
}
void UDolbyIoSubsystem::Connect(const FString& ConferenceName, const FString& UserName)
{
	CppSdk->Connect(ConferenceName, UserName);
}
void UDolbyIoSubsystem::Disconnect()
{
	CppSdk->Disconnect();
}
void UDolbyIoSubsystem::UpdateViewPoint(const FVector& Position, const FRotator& Rotation)
{
	if (SpatialUpdateTimerHandle.IsValid())
	{
		GameInstance->GetTimerManager().ClearTimer(SpatialUpdateTimerHandle);
	}

	CppSdk->UpdateViewPoint(Position, Rotation);
}
void UDolbyIoSubsystem::MuteInput()
{
	CppSdk->MuteInput();
}
void UDolbyIoSubsystem::UnmuteInput()
{
	CppSdk->UnmuteInput();
}
void UDolbyIoSubsystem::MuteOutput()
{
	CppSdk->MuteOutput();
}
void UDolbyIoSubsystem::UnmuteOutput()
{
	CppSdk->UnmuteOutput();
}
void UDolbyIoSubsystem::GetAudioLevels()
{
	CppSdk->GetAudioLevels();
}
void UDolbyIoSubsystem::SetInputDevice(int Index)
{
	CppSdk->SetInputDevice(Index);
}
void UDolbyIoSubsystem::SetOutputDevice(int Index)
{
	CppSdk->SetOutputDevice(Index);
}

void UDolbyIoSubsystem::UpdateViewPointUsingFirstPlayer()
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

void UDolbyIoSubsystem::OnTokenNeededEvent()
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnTokenNeeded(); });
}
void UDolbyIoSubsystem::OnInitializedEvent()
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnInitialized(); });
}
void UDolbyIoSubsystem::OnConnectedEvent()
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnConnected(); });
}
void UDolbyIoSubsystem::OnDisconnectedEvent()
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnDisconnected(); });
}
void UDolbyIoSubsystem::OnLocalParticipantChangedEvent(const Dolby::FParticipant& Participant)
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnLocalParticipantChanged(Participant); });
}
void UDolbyIoSubsystem::OnListOfRemoteParticipantsChangedEvent(const Dolby::FParticipants& Participants)
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnListOfRemoteParticipantsChanged(Participants); });
}
void UDolbyIoSubsystem::OnListOfActiveSpeakersChangedEvent(const Dolby::FParticipants& Speakers)
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnListOfActiveSpeakersChanged(Speakers); });
}
void UDolbyIoSubsystem::OnListOfAudioLevelsChangedEvent(const Dolby::FAudioLevels& Levels)
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnListOfAudioLevelsChanged(Levels); });
}
void UDolbyIoSubsystem::OnListOfInputDevicesChangedEvent(const Dolby::FDeviceNames& Devices)
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnListOfInputDevicesChanged(Devices); });
}
void UDolbyIoSubsystem::OnListOfOutputDevicesChangedEvent(const Dolby::FDeviceNames& Devices)
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnListOfOutputDevicesChanged(Devices); });
}
void UDolbyIoSubsystem::OnCurrentInputDeviceChangedEvent(int Index)
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnCurrentInputDeviceChanged(Index); });
}
void UDolbyIoSubsystem::OnCurrentOutputDeviceChangedEvent(int Index)
{
	AsyncTask(ENamedThreads::GameThread, [=] { OnCurrentOutputDeviceChanged(Index); });
}
