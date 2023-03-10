// Copyright 2023 Dolby Laboratories

#include "DolbyIOAsyncActions.h"

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

void UDolbyIOConnect::OnConnectedImpl(const FString& LocalParticipantID)
{
	OnConnected.Broadcast(LocalParticipantID);
}

UDolbyIOConnect* UDolbyIOConnect::DolbyIOConnect(const UObject* WorldContextObject, const FString& ConferenceName,
                                                 const FString& UserName, const FString& ExternalID,
                                                 const FString& AvatarURL)
{
	UDolbyIOConnect* Self = NewObject<UDolbyIOConnect>();
	Self->WorldContextObject = WorldContextObject;
	Self->ConferenceName = ConferenceName;
	Self->UserName = UserName;
	Self->ExternalID = ExternalID;
	Self->AvatarURL = AvatarURL;
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

void UDolbyIODemoConference::OnConnectedImpl(const FString& LocalParticipantID)
{
	OnConnected.Broadcast(LocalParticipantID);
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

UDolbyIODisconnect* UDolbyIODisconnect::DolbyIODisconnect(const UObject* WorldContextObject)
{
	UDolbyIODisconnect* Self = NewObject<UDolbyIODisconnect>();
	Self->WorldContextObject = WorldContextObject;
	return Self;
}
