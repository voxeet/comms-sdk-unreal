// Copyright 2023 Dolby Laboratories

#pragma once

#include "HAL/Platform.h"

#if PLATFORM_MAC
#define DOLBYIO_COMMS_SUPPRESS_APPLE_NO_RTTI_WARNING
#define __NUMBERFORMATTING__ // redefinition of 'FVector'
#endif

#include <dolbyio/comms/sdk.h>
#include <dolbyio/comms/utils/vfs_event.h>

#if PLATFORM_WINDOWS | PLATFORM_MAC
#include <dolbyio/comms/video_processor.h>
#endif
