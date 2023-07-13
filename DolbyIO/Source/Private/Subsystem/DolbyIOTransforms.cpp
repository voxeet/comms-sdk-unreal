// Copyright 2023 Dolby Laboratories

#include "DolbyIO.h"

#include "Utils/DolbyIOConversions.h"
#include "Utils/DolbyIOErrorHandler.h"
#include "Utils/DolbyIOLogging.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

using namespace DolbyIO;

void UDolbyIOSubsystem::SetLocalPlayerLocation(const FVector& Location)
{
	if (LocationTimerHandle.IsValid())
	{
		DLB_UE_LOG("Disabling automatic location setting");
		GetGameInstance()->GetTimerManager().ClearTimer(LocationTimerHandle);
	}
	SetLocalPlayerLocationImpl(Location);
}

void UDolbyIOSubsystem::SetLocalPlayerLocationImpl(const FVector& Location)
{
	if (!IsConnectedAsActive() || !IsSpatialAudio())
	{
		return;
	}

	Sdk->conference()
	    .set_spatial_position(ToStdString(LocalParticipantID), {Location.X, Location.Y, Location.Z})
	    .on_error(DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::SetLocalPlayerRotation(const FRotator& Rotation)
{
	if (RotationTimerHandle.IsValid())
	{
		DLB_UE_LOG("Disabling automatic rotation setting");
		GetGameInstance()->GetTimerManager().ClearTimer(RotationTimerHandle);
	}
	SetLocalPlayerRotationImpl(Rotation);
}

void UDolbyIOSubsystem::SetLocalPlayerRotationImpl(const FRotator& Rotation)
{
	if (!IsConnectedAsActive() || !IsSpatialAudio())
	{
		return;
	}

	// The SDK expects the direction values to mean rotations around the {x,y,z} axes as specified by the
	// environment. In Unreal, rotation around x is roll (because x is forward), y is pitch and z is yaw.
	Sdk->conference().set_spatial_direction({Rotation.Roll, Rotation.Pitch, Rotation.Yaw}).on_error(DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::SetRemotePlayerLocation(const FString& ParticipantID, const FVector& Location)
{
	if (!IsConnectedAsActive() || SpatialAudioStyle != EDolbyIOSpatialAudioStyle::Individual ||
	    ParticipantID == LocalParticipantID)
	{
		return;
	}

	Sdk->conference()
	    .set_spatial_position(ToStdString(ParticipantID), {Location.X, Location.Y, Location.Z})
	    .on_error(DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::SetLocationUsingFirstPlayer()
{
	if (UWorld* World = GetGameInstance()->GetWorld())
	{
		if (APlayerController* FirstPlayerController = World->GetFirstPlayerController())
		{
			if (APawn* Pawn = FirstPlayerController->GetPawn())
			{
				SetLocalPlayerLocationImpl(Pawn->GetActorLocation());
			}
		}
	}
}

void UDolbyIOSubsystem::SetRotationUsingFirstPlayer()
{
	if (UWorld* World = GetGameInstance()->GetWorld())
	{
		if (APlayerController* FirstPlayerController = World->GetFirstPlayerController())
		{
			if (APawn* Pawn = FirstPlayerController->GetPawn())
			{
				SetLocalPlayerRotationImpl(Pawn->GetActorRotation());
			}
		}
	}
}
