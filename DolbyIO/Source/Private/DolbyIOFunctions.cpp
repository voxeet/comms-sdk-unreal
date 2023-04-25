// Copyright 2023 Dolby Laboratories

#include "DolbyIOFunctions.h"

#include "DolbyIOSubsystem.h"

#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	UDolbyIOSubsystem* GetDolbyIOSubsystem(const UObject* WorldContextObject)
	{
		if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject))
		{
			return GameInstance->GetSubsystem<UDolbyIOSubsystem>();
		}
		return nullptr;
	}
}

void UDolbyIOSetToken::Activate()
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->OnInitialized.AddDynamic(this, &UDolbyIOSetToken::OnInitializedImpl);
		DolbyIOSubsystem->SetToken(Token);
	}
}

void UDolbyIOSetToken::OnInitializedImpl()
{
	OnInitialized.Broadcast();
}

UDolbyIOSetToken* UDolbyIOSetToken::DolbyIOSetToken(const UObject* WorldContextObject, const FString& Token)
{
	UDolbyIOSetToken* Self = NewObject<UDolbyIOSetToken>();
	Self->WorldContextObject = WorldContextObject;
	Self->Token = Token;
	return Self;
}

void UDolbyIOConnect::Activate()
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->OnConnected.AddDynamic(this, &UDolbyIOConnect::OnConnectedImpl);
		DolbyIOSubsystem->Connect(ConferenceName, UserName, ExternalID, AvatarURL);
	}
}

void UDolbyIOConnect::OnConnectedImpl(const FString& LocalParticipantID, const FString& ConferenceID)
{
	OnConnected.Broadcast(LocalParticipantID, ConferenceID);
}

UDolbyIOConnect* UDolbyIOConnect::DolbyIOConnect(const UObject* WorldContextObject, const FString& ConferenceName,
                                                 const FString& UserName, const FString& ExternalID,
                                                 const FString& AvatarURL, EDolbyIOConnectionMode ConnectionMode,
                                                 EDolbyIOSpatialAudioStyle SpatialAudioStyle)
{
	UDolbyIOConnect* Self = NewObject<UDolbyIOConnect>();
	Self->WorldContextObject = WorldContextObject;
	Self->ConferenceName = ConferenceName;
	Self->UserName = UserName;
	Self->ExternalID = ExternalID;
	Self->AvatarURL = AvatarURL;
	Self->ConnectionMode = ConnectionMode;
	Self->SpatialAudioStyle = SpatialAudioStyle;
	return Self;
}

void UDolbyIODemoConference::Activate()
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->OnConnected.AddDynamic(this, &UDolbyIODemoConference::OnConnectedImpl);
		DolbyIOSubsystem->DemoConference();
	}
}

void UDolbyIODemoConference::OnConnectedImpl(const FString& LocalParticipantID, const FString& ConferenceID)
{
	OnConnected.Broadcast(LocalParticipantID, ConferenceID);
}

UDolbyIODemoConference* UDolbyIODemoConference::DolbyIODemoConference(const UObject* WorldContextObject)
{
	UDolbyIODemoConference* Self = NewObject<UDolbyIODemoConference>();
	Self->WorldContextObject = WorldContextObject;
	return Self;
}

void UDolbyIODisconnect::Activate()
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->OnDisconnected.AddDynamic(this, &UDolbyIODisconnect::OnDisconnectedImpl);
		DolbyIOSubsystem->Disconnect();
	}
}

void UDolbyIODisconnect::OnDisconnectedImpl()
{
	OnDisconnected.Broadcast();
}

UDolbyIOGetScreenshareSources* UDolbyIOGetScreenshareSources::DolbyIOGetScreenshareSources(
    const UObject* WorldContextObject)
{
	UDolbyIOGetScreenshareSources* Self = NewObject<UDolbyIOGetScreenshareSources>();
	Self->WorldContextObject = WorldContextObject;
	return Self;
}

void UDolbyIOGetScreenshareSources::Activate()
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->OnScreenshareSourcesReceived.AddDynamic(
		    this, &UDolbyIOGetScreenshareSources::OnScreenshareSourcesReceivedImpl);
		DolbyIOSubsystem->GetScreenshareSources();
	}
}

void UDolbyIOGetScreenshareSources::OnScreenshareSourcesReceivedImpl(const TArray<FDolbyIOScreenshareSource>& Sources)
{
	OnScreenshareSourcesReceived.Broadcast(Sources);
}

UDolbyIODisconnect* UDolbyIODisconnect::DolbyIODisconnect(const UObject* WorldContextObject)
{
	UDolbyIODisconnect* Self = NewObject<UDolbyIODisconnect>();
	Self->WorldContextObject = WorldContextObject;
	return Self;
}

void UDolbyIOBlueprintFunctionLibrary::SetSpatialEnvironmentScale(const UObject* WorldContextObject,
                                                                  float SpatialEnvironmentScale)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->SetSpatialEnvironmentScale(SpatialEnvironmentScale);
	}
}
void UDolbyIOBlueprintFunctionLibrary::MuteInput(const UObject* WorldContextObject)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->MuteInput();
	}
}
void UDolbyIOBlueprintFunctionLibrary::UnmuteInput(const UObject* WorldContextObject)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->UnmuteInput();
	}
}
void UDolbyIOBlueprintFunctionLibrary::MuteOutput(const UObject* WorldContextObject)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->MuteOutput();
	}
}
void UDolbyIOBlueprintFunctionLibrary::UnmuteOutput(const UObject* WorldContextObject)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->UnmuteOutput();
	}
}
void UDolbyIOBlueprintFunctionLibrary::MuteParticipant(const UObject* WorldContextObject, const FString& ParticipantID)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->MuteParticipant(ParticipantID);
	}
}
void UDolbyIOBlueprintFunctionLibrary::UnmuteParticipant(const UObject* WorldContextObject,
                                                         const FString& ParticipantID)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->UnmuteParticipant(ParticipantID);
	}
}
void UDolbyIOBlueprintFunctionLibrary::EnableVideo(const UObject* WorldContextObject)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->EnableVideo();
	}
}
void UDolbyIOBlueprintFunctionLibrary::DisableVideo(const UObject* WorldContextObject)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->DisableVideo();
	}
}
void UDolbyIOBlueprintFunctionLibrary::BindMaterial(const UObject* WorldContextObject,
                                                    UMaterialInstanceDynamic* Material, const FString& VideoTrackID)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->BindMaterial(Material, VideoTrackID);
	}
}
void UDolbyIOBlueprintFunctionLibrary::UnbindMaterial(const UObject* WorldContextObject,
                                                      UMaterialInstanceDynamic* Material, const FString& VideoTrackID)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->UnbindMaterial(Material, VideoTrackID);
	}
}
UTexture2D* UDolbyIOBlueprintFunctionLibrary::GetTexture(const UObject* WorldContextObject, const FString& VideoTrackID)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		return DolbyIOSubsystem->GetTexture(VideoTrackID);
	}
	return nullptr;
}
void UDolbyIOBlueprintFunctionLibrary::StartScreenshare(const UObject* WorldContextObject,
                                                        const FDolbyIOScreenshareSource& Source,
                                                        EDolbyIOScreenshareContentType ContentType)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->StartScreenshare(Source, ContentType);
	}
}
void UDolbyIOBlueprintFunctionLibrary::StopScreenshare(const UObject* WorldContextObject)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->StopScreenshare();
	}
}
void UDolbyIOBlueprintFunctionLibrary::ChangeScreenshareContentType(const UObject* WorldContextObject,
                                                                    EDolbyIOScreenshareContentType ContentType)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->ChangeScreenshareContentType(ContentType);
	}
}
void UDolbyIOBlueprintFunctionLibrary::SetLocalPlayerLocation(const UObject* WorldContextObject,
                                                              const FVector& Location)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->SetLocalPlayerLocation(Location);
	}
}
void UDolbyIOBlueprintFunctionLibrary::SetLocalPlayerRotation(const UObject* WorldContextObject,
                                                              const FRotator& Rotation)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->SetLocalPlayerRotation(Rotation);
	}
}