// Copyright 2023 Dolby Laboratories

#pragma once

#include "HAL/Platform.h"

#if PLATFORM_WINDOWS
#pragma warning(push)
// 'type' : class 'type1' needs to have dll-interface to be used by clients of class 'type2'
#pragma warning(disable : 4251)
// non - DLL-interface class 'class_1' used as base for DLL-interface class 'class_2'
#pragma warning(disable : 4275)
#elif PLATFORM_MAC
#define DOLBYIO_COMMS_SUPPRESS_APPLE_NO_RTTI_WARNING
#define __NUMBERFORMATTING__ // redefinition of 'FVector'
#endif

#include <dolbyio/comms/sdk.h>

#if PLATFORM_WINDOWS
#pragma warning(pop)
#endif
