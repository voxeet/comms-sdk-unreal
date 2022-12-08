// Copyright 2022 Dolby Laboratories

#include "DolbyIoConference.h"

#include "SdkAccess.h"

#include "Async/Async.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_MODULE(FDefaultModuleImpl, DolbyIoConferenceModule)

ADolbyIoConference::ADolbyIoConference()
    : ConferenceName("unreal"), UserName("unreal"), Status("Disconnected"), CppSdk(MakeShared<Dolby::FSdkAccess>())
{
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.03;
}

void ADolbyIoConference::BeginPlay()
{
	Super::BeginPlay();

	if (const auto World = GetWorld())
	{
		FirstPlayerController = World->GetFirstPlayerController();
	}
	CppSdk->SetObserver(this);
	if (!Token.IsEmpty())
	{
		CppSdk->RefreshToken(Token);
	}
}

void ADolbyIoConference::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CppSdk->ShutDown();
	Super::EndPlay(EndPlayReason);
}

void ADolbyIoConference::Connect()
{
	CppSdk->Connect(Token, ConferenceName, UserName);
}
void ADolbyIoConference::Disconnect()
{
	CppSdk->Disconnect();
}
void ADolbyIoConference::MuteInput()
{
	CppSdk->MuteInput(bIsInputMuted);
}
void ADolbyIoConference::MuteOutput()
{
	CppSdk->MuteOutput(bIsOutputMuted);
}
void ADolbyIoConference::SetInputDevice(const int Index)
{
	CppSdk->SetInputDevice(Index);
}
void ADolbyIoConference::SetOutputDevice(const int Index)
{
	CppSdk->SetOutputDevice(Index);
}
void ADolbyIoConference::GetAudioLevels()
{
	CppSdk->GetAudioLevels();
}
void ADolbyIoConference::RefreshToken()
{
	if (Token != PreviousToken)
	{
		CppSdk->RefreshToken(Token);
		PreviousToken = Token;
	}
}

dolbyio::comms::sdk* ADolbyIoConference::GetRawSdk()
{
	return CppSdk->GetRawSdk();
}

void ADolbyIoConference::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	OnSpatialUpdateNeeded();
	CppSdk->UpdateViewPoint(Position, Rotation);
}

inline void ADolbyIoConference::TriggerEvent(void (ADolbyIoConference::*Function)())
{
	AsyncTask(ENamedThreads::GameThread, [this, Function] { (this->*Function)(); });
}

void ADolbyIoConference::OnStatusChanged(const Dolby::FMessage& Msg)
{
	Status = Msg;
	TriggerEvent(&ADolbyIoConference::OnStatusChanged);
}

void ADolbyIoConference::OnListOfInputDevicesChanged(const Dolby::FDeviceNames NewInputDevices)
{
	InputDevices = NewInputDevices;
	TriggerEvent(&ADolbyIoConference::OnNewListOfInputDevices);
}

void ADolbyIoConference::OnListOfOutputDevicesChanged(const Dolby::FDeviceNames NewOutputDevices)
{
	OutputDevices = NewOutputDevices;
	TriggerEvent(&ADolbyIoConference::OnNewListOfOutputDevices);
}

void ADolbyIoConference::OnInputDeviceChanged(const int Index)
{
	CurrentInputDeviceIndex = Index;
	TriggerEvent(&ADolbyIoConference::OnInputDeviceChanged);
}

void ADolbyIoConference::OnOutputDeviceChanged(const int Index)
{
	CurrentOutputDeviceIndex = Index;
	TriggerEvent(&ADolbyIoConference::OnOutputDeviceChanged);
}

void ADolbyIoConference::OnLocalParticipantChanged(const Dolby::FParticipant& Participant)
{
	LocalParticipant = Participant;
	TriggerEvent(&ADolbyIoConference::OnLocalParticipantChanged);
}

void ADolbyIoConference::OnListOfRemoteParticipantsChanged(const Dolby::FParticipants& NewListOfParticipants)
{
	RemoteParticipants = NewListOfParticipants;
	TriggerEvent(&ADolbyIoConference::OnNewListOfRemoteParticipants);
}

void ADolbyIoConference::OnListOfActiveSpeakersChanged(const Dolby::FParticipants Speakers)
{
	ActiveSpeakers = Speakers;
	TriggerEvent(&ADolbyIoConference::OnNewListOfActiveSpeakers);
}

void ADolbyIoConference::OnAudioLevelsChanged(const Dolby::FAudioLevels Levels)
{
	AudioLevels = Levels;
	TriggerEvent(&ADolbyIoConference::OnNewAudioLevels);
}

void ADolbyIoConference::OnRefreshTokenRequested()
{
	TriggerEvent(&ADolbyIoConference::OnRefreshTokenNeeded);
}

void ADolbyIoConference::OnSpatialUpdateNeeded_Implementation()
{
	if (FirstPlayerController)
	{
		FirstPlayerController->GetActorEyesViewPoint(Position, Rotation);
	}
}

void ADolbyIoConference::OnStatusChanged_Implementation() {}
void ADolbyIoConference::OnNewListOfInputDevices_Implementation() {}
void ADolbyIoConference::OnNewListOfOutputDevices_Implementation() {}
void ADolbyIoConference::OnInputDeviceChanged_Implementation() {}
void ADolbyIoConference::OnOutputDeviceChanged_Implementation() {}
void ADolbyIoConference::OnLocalParticipantChanged_Implementation() {}
void ADolbyIoConference::OnNewListOfRemoteParticipants_Implementation() {}
void ADolbyIoConference::OnNewListOfActiveSpeakers_Implementation() {}
void ADolbyIoConference::OnNewAudioLevels_Implementation() {}
void ADolbyIoConference::OnRefreshTokenNeeded_Implementation() {}
