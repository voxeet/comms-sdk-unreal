// Copyright 2023 Dolby Laboratories

#pragma once

#include "DolbyIOLogLevel.generated.h"

/** The level of logs of the Dolby.io C++ SDK. */
UENUM(BlueprintType, DisplayName = "Dolby.io Log Level")
enum class EDolbyIOLogLevel : uint8
{
	/** Turns off logging. */
	Off,

	/** The error level logging generates logs when an error occurs and the SDK cannot properly function. */
	Error,

	/** The warning level logging generates logs when the SDK detects an unexpected problem but is still able to work as
	 * usual.
	 */
	Warning,

	/** The info level logging generates an informative number of logs. */
	Info,

	/** The debug level logging generates a high number of logs to provide diagnostic information in a detailed manner.
	 */
	Debug,

	/** The verbose level logging generates the highest number of logs, including even the HTTP requests. */
	Verbose
};
