// Copyright 2022 Dolby Laboratories

#include "DolbyIO/Logging.h"

#include "HAL/PlatformProcess.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"

namespace DolbyIO
{
	class FDolbyIOModule final : public IModuleInterface
	{
	public:
		void StartupModule() override
		{
#if PLATFORM_WINDOWS
			LoadDll("bin/avutil-56.dll");
			LoadDll("bin/dvclient.dll");
			LoadDll("bin/dolbyio_comms_media.dll");
			LoadDll("bin/dolbyio_comms_sdk.dll");
#elif PLATFORM_MAC
			LoadDll("lib/libdolbyio_comms_media.dylib");
			LoadDll("lib/libdolbyio_comms_sdk.dylib");
			LoadDll("lib/libdvclient.dylib");
#endif
		}

		void ShutdownModule() override
		{
			for (auto Handle : DllHandles)
			{
				FPlatformProcess::FreeDllHandle(Handle);
			}
		}

	private:
		void LoadDll(const FString& Dll)
		{
			const static auto BaseDir = FPaths::Combine(*IPluginManager::Get().FindPlugin("DolbyIO")->GetBaseDir(),
			                                            TEXT("Source/ThirdParty/sdk-release"));
			if (const auto Handle = FPlatformProcess::GetDllHandle(*FPaths::Combine(*BaseDir, *Dll)))
			{
				DllHandles.Add(Handle);
			}
			else
			{
				UE_LOG(LogDolbyIO, Fatal, TEXT("Failed to load %s"), *Dll);
			}
		}

		TArray<void*> DllHandles;
	};
}

IMPLEMENT_MODULE(DolbyIO::FDolbyIOModule, DolbyIOModule)
DEFINE_LOG_CATEGORY(LogDolbyIO);
