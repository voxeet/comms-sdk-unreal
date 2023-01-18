// Copyright 2023 Dolby Laboratories

#pragma once

#include "DolbyIOBase64Functions.generated.h"

UCLASS()
class UBase64Functions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Creates a Base64-encoded string.
	 *
	 * @param Source - The string to encode.
	 * @return The encoded string.
	 */
	UFUNCTION(BlueprintCallable, Category = "Utilities|String", Meta = (DisplayName = "Encode Base64"))
	static FString EncodeBase64(const FString& Source);

	/** Decodes a Base64-encoded string.
	 *
	 * @param Source - The string to decode.
	 * @return The decoded string.
	 */
	UFUNCTION(BlueprintCallable, Category = "Utilities|String", Meta = (DisplayName = "Decode Base64"))
	static FString DecodeBase64(const FString& Source);
};
