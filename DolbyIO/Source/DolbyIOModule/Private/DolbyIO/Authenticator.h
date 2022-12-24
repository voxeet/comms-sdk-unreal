// Copyright 2022 Dolby Laboratories

#pragma once

#include "Interfaces/IHttpRequest.h"

class UDolbyIO;

namespace DolbyIO
{
	class FAuthenticator final
	{
	public:
		FAuthenticator(UDolbyIO&);

		void GetToken(const FString& AppKey, const FString& AppSecret, int TokenExpirationTimeInSeconds);

	private:
		void OnTokenObtained(FHttpRequestPtr, FHttpResponsePtr, bool bConnectedSuccessfully);

		UDolbyIO& DolbyIO;
	};
}
