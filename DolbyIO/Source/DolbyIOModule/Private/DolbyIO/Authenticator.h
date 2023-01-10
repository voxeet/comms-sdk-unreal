// Copyright 2022 Dolby Laboratories

#pragma once

#include "Interfaces/IHttpRequest.h"

class UDolbyIOSubsystem;

namespace DolbyIO
{
	class FAuthenticator final
	{
	public:
		FAuthenticator(UDolbyIOSubsystem&);

		void GetToken(const FString& AppKey, const FString& AppSecret, int TokenExpirationTimeInSeconds);

	private:
		void OnTokenObtained(FHttpRequestPtr, FHttpResponsePtr, bool bConnectedSuccessfully);

		UDolbyIOSubsystem& DolbyIOSubsystem;
	};
}
