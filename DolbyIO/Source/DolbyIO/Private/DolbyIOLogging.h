// Copyright 2023 Dolby Laboratories

#pragma once

#include "Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDolbyIO, Log, All);

#define DLB_UE_LOG(Category, Format, ...) UE_LOG(LogDolbyIO, Category, TEXT(Format), ##__VA_ARGS__)
