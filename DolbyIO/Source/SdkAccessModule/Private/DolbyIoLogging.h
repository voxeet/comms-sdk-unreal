// Copyright 2022 Dolby Laboratories

#pragma once

#include "Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDolby, Log, All);
#define DLB_UE_LOG(Format, ...) UE_LOG(LogDolby, Warning, TEXT(Format), ##__VA_ARGS__)
