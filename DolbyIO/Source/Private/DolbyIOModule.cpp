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
		// Add this here as I am not sure how Windows paths are interpreted (do I need the backslash)
		BaseDir = FPaths::Combine(BaseDir, TEXT("bin"));
		LoadDll(BaseDir, "avutil-57.dll");
		LoadDll(BaseDir, "avcodec-59.dll");
		LoadDll(BaseDir, "avformat-59.dll");
		LoadDll(BaseDir, "dvclient.dll");
		LoadDll(BaseDir, "dolbyio_comms_media.dll");
		LoadDll(BaseDir, "dolbyio_comms_sdk.dll");
		sdk::set_app_allocator(Allocator);
		LoadDll(BaseDir, "opencv_core451.dll");
		LoadDll(BaseDir, "opencv_imgproc451.dll");
		LoadDll(BaseDir, "opencv_imgcodecs451.dll");
		LoadDll(BaseDir, "dvdnr.dll");
		LoadDll(BaseDir, "dlb_vidseg_c_api.dll");
		LoadDll(BaseDir, "video_processor.dll");
		dolbyio::comms::plugin::video_processor::set_app_allocator(Allocator);
#elif PLATFORM_MAC
		LoadDll(BaseDir, "lib/libdvclient.dylib");
		LoadDll(BaseDir, "lib/libdolbyio_comms_media.dylib");
		LoadDll(BaseDir, "lib/libdolbyio_comms_sdk.dylib");
		LoadDll(BaseDir, "lib/libopencv_core.4.5.dylib");
		LoadDll(BaseDir, "lib/libopencv_imgproc.4.5.dylib");
		LoadDll(BaseDir, "lib/libopencv_imgcodecs.4.5.dylib");
		LoadDll(BaseDir, "lib/libdlb_vidseg_c_api.dylib");
		LoadDll(BaseDir, "lib/libdvdnr.dylib");
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
		while (Dlls.Num())
		{
			const FDll Dll = Dlls.Pop();
			FPlatformProcess::FreeDllHandle(Dll.Handle);
			DLB_UE_LOG("Unloaded %s", *Dll.Name);
		}
	}

private:
	void LoadDll(const FString& BaseDir, const FString& Dll)
	{
		const FString DllPath = FPaths::Combine(*BaseDir, *Dll);
		if (FDllHandle Handle = FPlatformProcess::GetDllHandle(*DllPath))
		{
			DLB_UE_LOG("Loaded %s", *Dll);
			Dlls.Emplace(FDll{Handle, Dll});
		}
		else
		{
			DLB_UE_LOG_BASE(Fatal, "Failed to load %s", *DllPath);
		}
	}

	struct FDll
	{
		FDllHandle Handle;
		FString Name;
	};

	TArray<FDll> Dlls;
};

IMPLEMENT_MODULE(FDolbyIOModule, DolbyIO)
DEFINE_LOG_CATEGORY(LogDolbyIO);
