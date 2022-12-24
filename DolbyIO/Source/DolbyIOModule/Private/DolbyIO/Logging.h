// Copyright 2022 Dolby Laboratories

#pragma once

#include "Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDolby, Log, All);
#define DLB_UE_LOG(Format, ...) UE_LOG(LogDolby, Log, TEXT(Format), ##__VA_ARGS__)
#define DLB_UE_LOG_ERROR(Format, ...) UE_LOG(LogDolby, Error, TEXT(Format), ##__VA_ARGS__)
