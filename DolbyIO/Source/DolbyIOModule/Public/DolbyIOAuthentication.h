// Copyright 2022 Dolby Laboratories

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "Delegates/Delegate.h"
#include "Interfaces/IHttpRequest.h"

#include "DolbyIOAuthentication.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGetDolbyIOTokenOutputPin, FString, Token);

UCLASS(ClassGroup = "Dolby.io Comms")
class DOLBYIOMODULE_API UGetDolbyIOToken : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/** For convenience during early development and prototyping, this function is provided to acquire the client access
	 * token directly from within the application. However, please note **we do not recommend** using this mechanism in
	 * the production software for [security best
	 * practices](https://docs.dolby.io/communications-apis/docs/guides-client-authentication). App secret needs to be
	 * protected and not included in the application.
	 *
	 * Example:
	 * <img src="https://files.readme.io/f38e604-set_token_using_key_and_secret.PNG">
	 * @param AppKey - The app key.
	 * @param AppSecret - The app secret.
	 * @param TokenExpirationTimeInSeconds - The token's expiration time (in seconds).
	 */
	UFUNCTION(BlueprintCallable,
	          meta = (BlueprintInternalUseOnly = "true", DisplayName = "Get Dolby.io Token",
	                  WorldContext = "WorldContextObject"),
	          Category = "Dolby.io Comms")
	static UGetDolbyIOToken* GetDolbyIOToken(UObject* WorldContextObject, const FString& AppKey,
	                                         const FString& AppSecret, int TokenExpirationTimeInSeconds = 3600);

	UPROPERTY(BlueprintAssignable)
	FGetDolbyIOTokenOutputPin TokenObtained;

private:
	void Activate() override;

	void OnTokenObtained(FHttpRequestPtr, FHttpResponsePtr, bool bConnectedSuccessfully);

	UObject* WorldContextObject;
	FString AppKey;
	FString AppSecret;
	int TokenExpirationTimeInSeconds;
};
