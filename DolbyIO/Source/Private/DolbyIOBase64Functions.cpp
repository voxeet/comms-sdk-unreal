// Copyright 2023 Dolby Laboratories

#include "DolbyIOBase64Functions.h"

#include "Misc/Base64.h"

FString UBase64Functions::EncodeBase64(const FString& Source)
{
	return FBase64::Encode(Source);
}

FString UBase64Functions::DecodeBase64(const FString& Source)
{
	FString Ret;
	FBase64::Decode(Source, Ret);
	return Ret;
}
