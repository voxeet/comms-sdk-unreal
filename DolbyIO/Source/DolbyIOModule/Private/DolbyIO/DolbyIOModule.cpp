// Copyright 2022 Dolby Laboratories

#include "DolbyIO/Logging.h"

#include <dolbyio/comms/sdk.h>

#include "HAL/PlatformProcess.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"

namespace DolbyIO
{
	class FDolbyIoSubsystemModule final : public IModuleInterface
	{
	public:
		void StartupModule() override
		try
		{
			using namespace dolbyio::comms;

#if PLATFORM_WINDOWS
			LoadDll("avutil-56.dll");
			LoadDll("dvclient.dll");
			LoadDll("dolbyio_comms_media.dll");
			LoadDll("dolbyio_comms_sdk.dll");

			static auto AlignedNew = +[](std::size_t Count, std::size_t Al)
			{ return operator new(Count, static_cast<std::align_val_t>(Al)); };
			static auto AlignedDelete =
			    +[](void* Ptr, std::size_t Al) { operator delete(Ptr, static_cast<std::align_val_t>(Al)); };
			sdk::set_app_allocator({operator new, AlignedNew, operator delete, AlignedDelete});
#endif
			sdk::log_settings LogSettings;
			LogSettings.sdk_log_level = log_level::INFO;
			LogSettings.media_log_level = log_level::OFF;
			LogSettings.log_directory = "";
			sdk::set_log_settings(LogSettings);
		}
		catch (const std::exception& ex)
		{
			DLB_UE_LOG("Error starting SdkAccessModule: %s", ex.what());
		}
		catch (...)
		{
			DLB_UE_LOG("Error starting SdkAccessModule: unknown");
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
			    FPaths::Combine(*IPluginManager::Get().FindPlugin("DolbyIO")->GetBaseDir(), TEXT("Binaries/Win64"));
			if (const auto Handle = FPlatformProcess::GetDllHandle(*FPaths::Combine(*BaseDir, *Dll)))
			{
				DllHandles.Add(Handle);
			}
			else
			{
				throw std::runtime_error{std::string{"Failed to load "} + TCHAR_TO_UTF8(*Dll)};
			}
		}

		TArray<void*> DllHandles;
	};
}

IMPLEMENT_MODULE(DolbyIO::FDolbyIoSubsystemModule, DolbyIoSubsystemModule)
DEFINE_LOG_CATEGORY(LogDolby);
