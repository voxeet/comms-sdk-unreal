// Copyright 2022 Dolby Laboratories

#pragma once

#include "Interfaces/IHttpRequest.h"

class UDolbyIoSubsystem;

namespace Dolby
{
	class FAuthenticator
	{
	public:
		FAuthenticator(UDolbyIoSubsystem&);

		void GetToken(const FString& AppKey, const FString& AppSecret, int TokenExpirationTimeInSeconds);

	private:
		void OnTokenObtained(FHttpRequestPtr, FHttpResponsePtr, bool bConnectedSuccessfully);

		UDolbyIoSubsystem& DolbyIoSubsystem;
	};
}
