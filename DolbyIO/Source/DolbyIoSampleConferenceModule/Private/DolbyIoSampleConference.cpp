// Copyright 2022 Dolby Laboratories

#include "DolbyIoSampleConference.h"

#include "Components/InputComponent.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_MODULE(FDefaultModuleImpl, DolbyIoSampleConferenceModule)

void ADolbyIoSampleConference::BeginPlay()
{
	Super::BeginPlay();

	if (!FirstPlayerController)
	{
		return;
	}

	InputComponent = NewObject<UInputComponent>(this);
	if (!InputComponent)
	{
		return;
	}

	InputComponent->RegisterComponent();
	InputComponent->BindKey(EKeys::X, IE_Pressed, this, &ADolbyIoSampleConference::Disconnect);
	InputComponent->BindKey(EKeys::C, IE_Pressed, this, &ADolbyIoConference::Connect);
	InputComponent->BindKey(EKeys::V, IE_Pressed, this, &ADolbyIoSampleConference::PreviousInputDevice);
	InputComponent->BindKey(EKeys::B, IE_Pressed, this, &ADolbyIoSampleConference::NextInputDevice);
	InputComponent->BindKey(EKeys::N, IE_Pressed, this, &ADolbyIoSampleConference::PreviousOutputDevice);
	InputComponent->BindKey(EKeys::M, IE_Pressed, this, &ADolbyIoSampleConference::NextOutputDevice);
	InputComponent->BindKey(EKeys::I, IE_Pressed, this, &ADolbyIoSampleConference::MuteInput);
	InputComponent->BindKey(EKeys::O, IE_Pressed, this, &ADolbyIoSampleConference::MuteOutput);
	InputComponent->BindKey(EKeys::L, IE_Pressed, this, &ADolbyIoConference::GetAudioLevels);
	EnableInput(FirstPlayerController);
}

void ADolbyIoSampleConference::OnNewListOfInputDevices_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("New list of input devices:"));
	for (const auto& Device : InputDevices)
	{
		UE_LOG(LogTemp, Warning, TEXT("\t%s"), *Device.ToString());
	}
}

void ADolbyIoSampleConference::OnNewListOfOutputDevices_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("New list of output devices:"));
	for (const auto& Device : OutputDevices)
	{
		UE_LOG(LogTemp, Warning, TEXT("\t%s"), *Device.ToString());
	}
}

void ADolbyIoSampleConference::OnLocalParticipantChanged_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Local participant ID: %s"), *LocalParticipant);
}

void ADolbyIoSampleConference::OnNewListOfRemoteParticipants_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("New list of remote participants:"));
	for (const auto& Participant : RemoteParticipants)
	{
		UE_LOG(LogTemp, Warning, TEXT("\t%s"), *Participant);
	}
}

void ADolbyIoSampleConference::OnNewListOfActiveSpeakers_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("New list of active speakers:"));
	for (const auto& Speaker : ActiveSpeakers)
	{
		UE_LOG(LogTemp, Warning, TEXT("\t%s"), *Speaker);
	}
}

void ADolbyIoSampleConference::OnNewAudioLevels_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("New audio levels:"));
	for (const auto& Level : AudioLevels)
	{
		UE_LOG(LogTemp, Warning, TEXT("\t%s = %f"), *Level.Key, Level.Value);
	}
}

void ADolbyIoSampleConference::OnRefreshTokenNeeded_Implementation()
{
	Super::RefreshToken();
}

void ADolbyIoSampleConference::Disconnect()
{
	Super::Disconnect();
	bIsInputMuted = bIsOutputMuted = false;
}

void ADolbyIoSampleConference::MuteInput()
{
	bIsInputMuted = !bIsInputMuted;
	Super::MuteInput();
}

void ADolbyIoSampleConference::MuteOutput()
{
	bIsOutputMuted = !bIsOutputMuted;
	Super::MuteOutput();
}

void ADolbyIoSampleConference::PreviousInputDevice()
{
	if (InputDevices.Num() < 2)
	{
		return;
	}
	const int Index = GetCurrentInputDeviceIndex() - 1;
	SetInputDevice(Index >= 0 ? Index : InputDevices.Num() - 1);
}

void ADolbyIoSampleConference::NextInputDevice()
{
	if (InputDevices.Num() < 2)
	{
		return;
	}
	const int Index = GetCurrentInputDeviceIndex() + 1;
	SetInputDevice(Index < InputDevices.Num() ? Index : 0);
}

void ADolbyIoSampleConference::PreviousOutputDevice()
{
	if (OutputDevices.Num() < 2)
	{
		return;
	}
	const int Index = GetCurrentOutputDeviceIndex() - 1;
	SetOutputDevice(Index >= 0 ? Index : OutputDevices.Num() - 1);
}

void ADolbyIoSampleConference::NextOutputDevice()
{
	if (OutputDevices.Num() < 2)
	{
		return;
	}
	const int Index = GetCurrentOutputDeviceIndex() + 1;
	SetOutputDevice(Index < OutputDevices.Num() ? Index : 0);
}

int ADolbyIoSampleConference::GetCurrentInputDeviceIndex() const
{
	return InputDevices.IndexOfByPredicate([this](const auto& Name) { return Name.EqualTo(CurrentInputDevice); });
}

int ADolbyIoSampleConference::GetCurrentOutputDeviceIndex() const
{
	return OutputDevices.IndexOfByPredicate([this](const auto& Name) { return Name.EqualTo(CurrentOutputDevice); });
}
