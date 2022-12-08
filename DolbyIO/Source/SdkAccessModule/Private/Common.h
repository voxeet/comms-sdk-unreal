// Copyright 2022 Dolby Laboratories

#pragma once

#if PLATFORM_MAC
#define DOLBYIO_COMMS_SUPPRESS_APPLE_NO_RTTI_WARNING
#endif
#include <dolbyio/comms/sdk.h>

#include "Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDolby, Log, All);
#define DLB_UE_LOG(Format, ...) UE_LOG(LogDolby, Log, TEXT(Format), ##__VA_ARGS__)

#include "CommonTypes.h"
