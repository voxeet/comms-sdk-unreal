// Copyright 2023 Dolby Laboratories

#include "Utils/DolbyIOCppSdk.h"
#include "Utils/DolbyIOLogging.h"

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
		using namespace dolbyio::comms;
		app_allocator Allocator{
		    ::operator new,
		    [](std::size_t Count, std::size_t Al) { return ::operator new(Count, static_cast<std::align_val_t>(Al)); },
		    ::operator delete,
		    [](void* Ptr, std::size_t Al) { ::operator delete(Ptr, static_cast<std::align_val_t>(Al)); }};
		LoadDll(BaseDir, "bin/avutil-57.dll");
		LoadDll(BaseDir, "bin/avcodec-59.dll");
		LoadDll(BaseDir, "bin/dvclient.dll");
		LoadDll(BaseDir, "bin/dolbyio_comms_media.dll");
		LoadDll(BaseDir, "bin/dolbyio_comms_sdk.dll");
		sdk::set_app_allocator(Allocator);
		LoadDll(BaseDir, "bin/opencv_core451.dll");
		LoadDll(BaseDir, "bin/opencv_imgproc451.dll");
		LoadDll(BaseDir, "bin/opencv_imgcodecs451.dll");
		LoadDll(BaseDir, "bin/dvdnr.dll");
		LoadDll(BaseDir, "bin/dlb_vidseg_c_api.dll");
		LoadDll(BaseDir, "bin/video_processor.dll");
		dolbyio::comms::plugin::video_processor::set_app_allocator(Allocator);
#elif PLATFORM_MAC
		LoadDll(BaseDir, "lib/libdolbyio_comms_media.dylib");
		LoadDll(BaseDir, "lib/libdolbyio_comms_sdk.dylib");
		LoadDll(BaseDir, "lib/libvideo_processor.dylib");
#elif PLATFORM_LINUX
		BaseDir += "-ubuntu-20.04-clang10-libc++10";
		LoadDll(BaseDir, "lib/libavutil.so.57");
		LoadDll(BaseDir, "lib/libavcodec.so.59");
		LoadDll(BaseDir, "lib/libavformat.so.59");
		LoadDll(BaseDir, "lib/libdvclient.so");
		LoadDll(BaseDir, "lib/libdolbyio_comms_media.so");
		LoadDll(BaseDir, "lib/libdolbyio_comms_sdk.so");
#endif
	}

	void ShutdownModule() override
	{
		while (DllHandles.Num())
		{
			FPlatformProcess::FreeDllHandle(DllHandles.Pop());
		}
	}

private:
	void LoadDll(const FString& BaseDir, const FString& Dll)
	{
		const FString DllPath = FPaths::Combine(*BaseDir, *Dll);
		if (FDllHandle Handle = FPlatformProcess::GetDllHandle(*DllPath))
		{
			DLB_UE_LOG("Loaded %s", *DllPath);
			DllHandles.Add(Handle);
		}
		else
		{
			DLB_UE_FATAL("Failed to load %s", *DllPath);
		}
	}

	TArray<FDllHandle> DllHandles;
};

IMPLEMENT_MODULE(FDolbyIOModule, DolbyIO)
DEFINE_LOG_CATEGORY(LogDolbyIO);
