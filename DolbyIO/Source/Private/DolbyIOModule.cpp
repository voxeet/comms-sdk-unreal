// Copyright 2023 Dolby Laboratories

#include "DolbyIOLogging.h"

#include "HAL/PlatformProcess.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"

class FDolbyIOModule final : public IModuleInterface
{
	using FDllHandle = void*;

public:
	void StartupModule() override
	{
		FString BaseDir =
		    FPaths::Combine(*IPluginManager::Get().FindPlugin("DolbyIO")->GetBaseDir(), TEXT("sdk-release"));
#if PLATFORM_WINDOWS
		LoadDll(BaseDir, "bin/avutil-57.dll");
		LoadDll(BaseDir, "bin/dvclient.dll");
		LoadDll(BaseDir, "bin/dolbyio_comms_media.dll");
		LoadDll(BaseDir, "bin/dolbyio_comms_sdk.dll");
#elif PLATFORM_MAC
		BaseDir += "-x86";
		LoadDll(BaseDir, "lib/libdolbyio_comms_media.dylib");
		LoadDll(BaseDir, "lib/libdolbyio_comms_sdk.dylib");
		LoadDll(BaseDir, "lib/libdvclient.dylib");
#endif
	}

	void ShutdownModule() override
	{
		for (FDllHandle Handle : DllHandles)
		{
			FPlatformProcess::FreeDllHandle(Handle);
		}
	}

private:
	void LoadDll(const FString& BaseDir, const FString& Dll)
	{
		if (FDllHandle Handle = FPlatformProcess::GetDllHandle(*FPaths::Combine(*BaseDir, *Dll)))
		{
			DllHandles.Add(Handle);
		}
		else
		{
			DLB_UE_LOG(Fatal, "Failed to load %s/%s", *BaseDir, *Dll);
		}
	}

	TArray<FDllHandle> DllHandles;
};

IMPLEMENT_MODULE(FDolbyIOModule, DolbyIO)
DEFINE_LOG_CATEGORY(LogDolbyIO);
