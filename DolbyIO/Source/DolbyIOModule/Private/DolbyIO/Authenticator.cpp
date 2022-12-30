// Copyright 2022 Dolby Laboratories

#include "DolbyIO/Authenticator.h"

#include "DolbyIO/Logging.h"
#include "DolbyIOSubsystem.h"

#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/Base64.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace DolbyIO
{
	FAuthenticator::FAuthenticator(UDolbyIOSubsystem& DolbyIOSubsystem) : DolbyIOSubsystem(DolbyIOSubsystem) {}

	void FAuthenticator::GetToken(const FString& AppKey, const FString& AppSecret, int TokenExpirationTimeInSeconds)
	{
		if (AppKey.IsEmpty() || AppSecret.IsEmpty())
		{
			UE_LOG(LogDolbyIO, Warning, TEXT("App key and secret must not be empty"));
			return;
		}
		if (TokenExpirationTimeInSeconds <= 0)
		{
			UE_LOG(LogDolbyIO, Warning, TEXT("Token expiration time must be greater than zero"));
			return;
		}

		FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
		Request->SetURL("https://session.voxeet.com/v1/oauth2/token");
		Request->SetVerb("POST");
		Request->AppendToHeader("Authorization", "Basic " + FBase64::Encode(AppKey + ":" + AppSecret));
		Request->AppendToHeader("Content-Type", "application/x-www-form-urlencoded");
		Request->SetContentAsString("grant_type=client_credentials&expires_in=" +
		                            FString::FromInt(TokenExpirationTimeInSeconds));
		Request->OnProcessRequestComplete().BindRaw(this, &FAuthenticator::OnTokenObtained);
		Request->ProcessRequest();
	}

	void FAuthenticator::OnTokenObtained(FHttpRequestPtr, FHttpResponsePtr Response, bool bConnectedSuccessfully)
	{
		if (!bConnectedSuccessfully)
		{
			UE_LOG(LogDolbyIO, Error, TEXT("Could not connect to backend serving access tokens"));
			return;
		}

		TSharedPtr<FJsonObject> ResponseObj;
		FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Response->GetContentAsString()), ResponseObj);
		FString Token;
		if (ResponseObj->TryGetStringField("access_token", Token))
		{
			DolbyIOSubsystem.SetToken(Token);
		}
		else
		{
			UE_LOG(LogDolbyIO, Error, TEXT("Could not get access token - verify app key and secret and validity"));
		}
	}
}
