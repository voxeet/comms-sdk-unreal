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
			LoadDll("avutil-56.dll");
			LoadDll("dvclient.dll");
			LoadDll("dolbyio_comms_media.dll");
			LoadDll("dolbyio_comms_sdk.dll");
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
			const static auto BaseDir =
			    FPaths::Combine(*IPluginManager::Get().FindPlugin("DolbyIO")->GetBaseDir(), TEXT("Source/ThirdParty/sdk-release/bin"));
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
