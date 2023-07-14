// Copyright 2023 Dolby Laboratories

#pragma once

#include "Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDolbyIO, Log, All);

#define DLB_UE_DEBUG(Format, ...) UE_LOG(LogDolbyIO, Verbose, TEXT(Format), ##__VA_ARGS__)
#define DLB_UE_LOG(Format, ...) UE_LOG(LogDolbyIO, Log, TEXT(Format), ##__VA_ARGS__)
#define DLB_UE_WARN(Format, ...) UE_LOG(LogDolbyIO, Warning, TEXT(Format), ##__VA_ARGS__)
#define DLB_UE_ERROR(Format, ...) UE_LOG(LogDolbyIO, Error, TEXT(Format), ##__VA_ARGS__)
#define DLB_UE_FATAL(Format, ...) UE_LOG(LogDolbyIO, Fatal, TEXT(Format), ##__VA_ARGS__)