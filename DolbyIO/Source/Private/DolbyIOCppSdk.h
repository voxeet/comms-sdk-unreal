// Copyright 2023 Dolby Laboratories

#pragma once

#include "HAL/Platform.h"

#if PLATFORM_MAC
#define DOLBYIO_COMMS_SUPPRESS_APPLE_NO_RTTI_WARNING
#define __NUMBERFORMATTING__ // redefinition of 'FVector'
#endif

#include <dolbyio/comms/sdk.h>
