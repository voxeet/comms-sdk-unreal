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
	    .on_error(DLB_ERROR_HANDLER(OnSetLocalPlayerLocationError));

#if PLATFORM_ANDROID
	GetGameInstance()->GetTimerManager().SetTimer(LocationTimerHandle, this,
	                                              &UDolbyIOSubsystem::SetLocationUsingFirstPlayer, 0.1, true);
#endif
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
	Sdk->conference()
	    .set_spatial_direction({Rotation.Roll, Rotation.Pitch, Rotation.Yaw})
	    .on_error(DLB_ERROR_HANDLER(OnSetLocalPlayerRotationError));

#if PLATFORM_ANDROID
	GetGameInstance()->GetTimerManager().SetTimer(RotationTimerHandle, this,
	                                              &UDolbyIOSubsystem::SetRotationUsingFirstPlayer, 0.01, true);
#endif
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
	    .on_error(DLB_ERROR_HANDLER(OnSetRemotePlayerLocationError));
}

namespace
{
	APawn* GetFirstPlayerPawn(UGameInstance* GameInstance)
	{
		if (UWorld* World = GameInstance->GetWorld())
		{
			if (APlayerController* FirstPlayerController = World->GetFirstPlayerController())
			{
				return FirstPlayerController->GetPawn();
			}
		}
		return nullptr;
	}
}

void UDolbyIOSubsystem::SetLocationUsingFirstPlayer()
{
	if (APawn* Pawn = GetFirstPlayerPawn(GetGameInstance()))
	{
		SetLocalPlayerLocationImpl(Pawn->GetActorLocation());
	}
}

void UDolbyIOSubsystem::SetRotationUsingFirstPlayer()
{
	if (APawn* Pawn = GetFirstPlayerPawn(GetGameInstance()))
	{
		SetLocalPlayerRotationImpl(Pawn->GetActorRotation());
	}
}
