// Copyright 2023 Dolby Laboratories

#include "DolbyIOAuthentication.h"

#include "Utils/DolbyIOLogging.h"

#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/Base64.h"
#include "Serialization/JsonSerializer.h"

namespace
{
	bool TryBroadcastToken(const FHttpResponsePtr& Response, const FGetDolbyIOTokenOutputPin& Delegate)
	{
		TSharedPtr<FJsonObject> ResponseObj;
		FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Response->GetContentAsString()), ResponseObj);
		FString Token;
		if (!ResponseObj->TryGetStringField("access_token", Token))
		{
			return false;
		}
		Delegate.Broadcast(Token, "");
		return true;
	}

	void BroadcastError(const FGetDolbyIOTokenOutputPin& Delegate, const FString& ErrorMsg)
	{
		DLB_UE_LOG_BASE(Error, "%s", *ErrorMsg);
		Delegate.Broadcast("", ErrorMsg);
	}
}

void UDolbyIOGetTokenFromURL::Activate()
{
	if (URL.IsEmpty())
	{
		return BroadcastError(OnError, "URL must not be empty");
	}

	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(URL);
	Request->SetVerb("GET");
	Request->AppendToHeader("Content-Type", "application/x-www-form-urlencoded");
	Request->OnProcessRequestComplete().BindUObject(this, &UDolbyIOGetTokenFromURL::OnTokenObtainedImpl);
	Request->ProcessRequest();
}

void UDolbyIOGetTokenFromURL::OnTokenObtainedImpl(FHttpRequestPtr, FHttpResponsePtr Response,
                                                  bool bConnectedSuccessfully)
{
	if (!bConnectedSuccessfully)
	{
		return BroadcastError(OnError, "Could not connect to given URL");
	}
	if (!TryBroadcastToken(Response, OnTokenObtained))
	{
		BroadcastError(OnError, "Could not get access token - no token in response from URL");
	}
}

UDolbyIOGetTokenFromURL* UDolbyIOGetTokenFromURL::DolbyIOGetTokenFromURL(const FString& URL)
{
	UDolbyIOGetTokenFromURL* Self = NewObject<UDolbyIOGetTokenFromURL>();
	Self->URL = URL;
	return Self;
}

void UGetDolbyIOToken::Activate()
{
	if (AppKey.IsEmpty() || AppSecret.IsEmpty())
	{
		return BroadcastError(OnError, "App key and secret must not be empty");
	}
	if (TokenExpirationTimeInSeconds <= 0)
	{
		return BroadcastError(OnError, "Token expiration time must be greater than zero");
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
		return BroadcastError(OnError, "Could not connect to backend serving access tokens");
	}
	if (!TryBroadcastToken(Response, TokenObtained))
	{
		BroadcastError(OnError, "Could not get access token - verify app key and secret and validity");
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
