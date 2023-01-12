// Copyright 2023 Dolby Laboratories

#include "DolbyIOSubsystem.h"

#include "DolbyIOSdkAccess.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

void UDolbyIOSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CppSdk = MakeShared<DolbyIO::FSdkAccess>(*this);

	auto& TimerManager = GetGameInstance()->GetTimerManager();
	TimerManager.SetTimer(LocationTimerHandle, this, &UDolbyIOSubsystem::SetLocationUsingFirstPlayer, 0.1, true);
	TimerManager.SetTimer(RotationTimerHandle, this, &UDolbyIOSubsystem::SetRotationUsingFirstPlayer, 0.01, true);

	OnTokenNeeded.Broadcast();
}

void UDolbyIOSubsystem::SetToken(const FString& Token)
{
	CppSdk->SetToken(Token);
}
void UDolbyIOSubsystem::Connect(const FString& ConferenceName, const FString& UserName, const FString& ExternalID,
                                const FString& AvatarURL)
{
	CppSdk->Connect(ConferenceName, UserName, ExternalID, AvatarURL);
}
void UDolbyIOSubsystem::DemoConference()
{
	CppSdk->DemoConference();
}
void UDolbyIOSubsystem::Disconnect()
{
	CppSdk->Disconnect();
}
void UDolbyIOSubsystem::SetSpatialEnvironmentScale(float SpatialEnvironmentScale)
{
	CppSdk->SetSpatialEnvironmentScale(SpatialEnvironmentScale);
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
void UDolbyIOSubsystem::SetLocalPlayerLocation(const FVector& Location)
{
	if (LocationTimerHandle.IsValid())
	{
		GetGameInstance()->GetTimerManager().ClearTimer(LocationTimerHandle);
	}
	CppSdk->SetLocalPlayerLocation(Location);
}
void UDolbyIOSubsystem::SetLocalPlayerRotation(const FRotator& Rotation)
{
	if (RotationTimerHandle.IsValid())
	{
		GetGameInstance()->GetTimerManager().ClearTimer(RotationTimerHandle);
	}
	CppSdk->SetLocalPlayerRotation(Rotation);
}
void UDolbyIOSubsystem::SetLocationUsingFirstPlayer()
{
	if (const auto World = GetGameInstance()->GetWorld())
	{
		if (const auto FirstPlayerController = World->GetFirstPlayerController())
		{
			CppSdk->SetLocalPlayerLocation(FirstPlayerController->GetPawn()->GetActorLocation());
		}
	}
}
void UDolbyIOSubsystem::SetRotationUsingFirstPlayer()
{
	if (const auto World = GetGameInstance()->GetWorld())
	{
		if (const auto FirstPlayerController = World->GetFirstPlayerController())
		{
			CppSdk->SetLocalPlayerRotation(FirstPlayerController->GetPawn()->GetActorRotation());
		}
	}
}
