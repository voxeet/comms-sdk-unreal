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
	const auto MaxIndex = InputDevices.Num() - 1;
	if (MaxIndex > 0)
	{
		SetInputDevice(CurrentInputDeviceIndex > 0 ? CurrentInputDeviceIndex - 1 : MaxIndex);
	}
}

void ADolbyIoSampleConference::NextInputDevice()
{
	const auto MaxIndex = InputDevices.Num() - 1;
	if (MaxIndex > 0)
	{
		SetInputDevice(CurrentInputDeviceIndex < MaxIndex ? CurrentInputDeviceIndex + 1 : 0);
	}
}

void ADolbyIoSampleConference::PreviousOutputDevice()
{
	const auto MaxIndex = OutputDevices.Num() - 1;
	if (MaxIndex > 0)
	{
		SetOutputDevice(CurrentOutputDeviceIndex > 0 ? CurrentOutputDeviceIndex - 1 : MaxIndex);
	}
}

void ADolbyIoSampleConference::NextOutputDevice()
{
	const auto MaxIndex = OutputDevices.Num() - 1;
	if (MaxIndex > 0)
	{
		SetOutputDevice(CurrentOutputDeviceIndex < MaxIndex ? CurrentOutputDeviceIndex + 1 : 0);
	}
}
