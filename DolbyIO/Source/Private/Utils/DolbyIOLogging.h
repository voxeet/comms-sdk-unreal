// Copyright 2023 Dolby Laboratories

#pragma once

#include "Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDolbyIO, Log, All);

#define DLB_UE_LOG_BASE(Verbosity, Format, ...) UE_LOG(LogDolbyIO, Verbosity, TEXT(Format), ##__VA_ARGS__)
#define DLB_UE_LOG(Format, ...) DLB_UE_LOG_BASE(Log, Format, ##__VA_ARGS__)
