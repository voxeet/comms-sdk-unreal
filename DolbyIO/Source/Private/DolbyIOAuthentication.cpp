// Copyright 2023 Dolby Laboratories

#include "DolbyIOAuthentication.h"

#include "DolbyIOLogging.h"

#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/Base64.h"
#include "Serialization/JsonSerializer.h"

void UGetDolbyIOToken::Activate()
{
	if (AppKey.IsEmpty() || AppSecret.IsEmpty())
	{
		DLB_UE_WARN("App key and secret must not be empty");
		return;
	}
	if (TokenExpirationTimeInSeconds <= 0)
	{
		DLB_UE_WARN("Token expiration time must be greater than zero");
		return;
	}

	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->SetURL("https://session.voxeet.com/v1/oauth2/token");
	Request->SetVerb("POST");
	Request->AppendToHeader("Authorization", "Basic " + FBase64::Encode(AppKey + ":" + AppSecret));
	Request->AppendToHeader("Content-Type", "application/x-www-form-urlencoded");
	Request->SetContentAsString("grant_type=client_credentials&expires_in=" +
	                            FString::FromInt(TokenExpirationTimeInSeconds));
	Request->OnProcessRequestComplete().BindUObject(this, &UGetDolbyIOToken::OnTokenObtained);
	Request->ProcessRequest();
}

void UGetDolbyIOToken::OnTokenObtained(FHttpRequestPtr, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	if (!bConnectedSuccessfully)
	{
		DLB_UE_ERROR("Could not connect to backend serving access tokens");
		return;
	}

	TSharedPtr<FJsonObject> ResponseObj;
	FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Response->GetContentAsString()), ResponseObj);
	FString Token;
	if (ResponseObj->TryGetStringField("access_token", Token))
	{
		TokenObtained.Broadcast(Token);
	}
	else
	{
		DLB_UE_ERROR("Could not get access token - verify app key and secret and validity");
	}
}

UGetDolbyIOToken* UGetDolbyIOToken::GetDolbyIOToken(const FString& AppKey, const FString& AppSecret,
                                                    int TokenExpirationTimeInSeconds)
{
	UGetDolbyIOToken* Self = NewObject<UGetDolbyIOToken>();
	Self->AppKey = AppKey;
	Self->AppSecret = AppSecret;
	Self->TokenExpirationTimeInSeconds = TokenExpirationTimeInSeconds;
	return Self;
}
