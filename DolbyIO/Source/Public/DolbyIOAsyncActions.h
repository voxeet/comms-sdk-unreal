// Copyright 2023 Dolby Laboratories

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "DolbyIOAsyncActions.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDolbyIOSetTokenOutputPin);

UCLASS()
class DOLBYIO_API UDolbyIOSetToken : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms",
	          Meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject",
	                  DisplayName = "Dolby.io Set Token"))
	static UDolbyIOSetToken* DolbyIOSetToken(const UObject* WorldContextObject, const FString& Token);

	UPROPERTY(BlueprintAssignable)
	FDolbyIOSetTokenOutputPin OnInitialized;

private:
	void Activate() override;

	UFUNCTION()
	void OnInitializedImpl();

	const UObject* WorldContextObject;
	FString Token;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDolbyIOConnectOutputPin, const FString&, LocalParticipantID);

UCLASS()
class DOLBYIO_API UDolbyIOConnect : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms",
	          Meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject",
	                  DisplayName = "Dolby.io Connect"))
	static UDolbyIOConnect* DolbyIOConnect(const UObject* WorldContextObject, const FString& ConferenceName = "unreal",
	                                       const FString& UserName = "", const FString& ExternalID = "",
	                                       const FString& AvatarURL = "");

	UPROPERTY(BlueprintAssignable)
	FDolbyIOConnectOutputPin OnConnected;

private:
	void Activate() override;

	UFUNCTION()
	void OnConnectedImpl(const FString& LocalParticipantID);

	const UObject* WorldContextObject;
	FString ConferenceName;
	FString UserName;
	FString ExternalID;
	FString AvatarURL;
};

UCLASS()
class DOLBYIO_API UDolbyIODemoConference : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms",
	          Meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject",
	                  DisplayName = "Dolby.io Demo Conference"))
	static UDolbyIODemoConference* DolbyIODemoConference(const UObject* WorldContextObject);

	UPROPERTY(BlueprintAssignable)
	FDolbyIOConnectOutputPin OnConnected;

private:
	void Activate() override;

	UFUNCTION()
	void OnConnectedImpl(const FString& LocalParticipantID);

	const UObject* WorldContextObject;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDolbyIODisconnectOutputPin);

UCLASS()
class DOLBYIO_API UDolbyIODisconnect : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms",
	          Meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject",
	                  DisplayName = "Dolby.io Disconnect"))
	static UDolbyIODisconnect* DolbyIODisconnect(const UObject* WorldContextObject);

	UPROPERTY(BlueprintAssignable)
	FDolbyIODisconnectOutputPin OnDisconnected;

private:
	void Activate() override;

	UFUNCTION()
	void OnDisconnectedImpl();

	const UObject* WorldContextObject;
};
