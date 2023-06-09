// Copyright 2023 Dolby Laboratories

using System.IO;
using UnrealBuildTool;

public class DolbyIO : ModuleRules
{
    public DolbyIO(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        CppStandard = CppStandardVersion.Cpp17;
        bEnableExceptions = true;

        PublicDependencyModuleNames.AddRange(
            new string[] { "Core", "CoreUObject", "Engine", "HTTP", "Json", "Projects", "RenderCore" });

        string ReleaseDir = "sdk-release";
        if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            ReleaseDir += "-ubuntu-20.04-clang10-libc++10";
        }
        string SdkDir = Path.Combine("$(PluginDir)", ReleaseDir);
        PublicIncludePaths.Add(Path.Combine(SdkDir, "include"));
        string LibDir = Path.Combine(SdkDir, "lib");

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicAdditionalLibraries.AddRange(new string[] {
                Path.Combine(LibDir, "dolbyio_comms_media.lib"),
                Path.Combine(LibDir, "dolbyio_comms_sdk.lib"),
            });

            string[] Dlls = new string[] { "avutil-57.dll", "avcodec-59.dll", "dvclient.dll", "dolbyio_comms_media.dll", "dolbyio_comms_sdk.dll" };
            PublicDelayLoadDLLs.AddRange(Dlls);

            string BinDir = Path.Combine(SdkDir, "bin");
            foreach (string Dll in Dlls)
            {
                RuntimeDependencies.Add(Path.Combine(BinDir, Dll));
            }
            RuntimeDependencies.Add(Path.Combine(BinDir, "cac_dvdnr.dll"));
            RuntimeDependencies.Add(Path.Combine(BinDir, "model.dnr"));
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            string[] Libs = new string[] { Path.Combine(LibDir, "libdvclient.dylib"),
                                           Path.Combine(LibDir, "libdolbyio_comms_media.dylib"),
                                           Path.Combine(LibDir, "libdolbyio_comms_sdk.dylib") };
            PublicAdditionalLibraries.AddRange(Libs);
            PublicDelayLoadDLLs.AddRange(Libs);

            foreach (string Lib in Libs)
            {
                RuntimeDependencies.Add(Lib);
            }
            RuntimeDependencies.Add(Path.Combine(LibDir, "libcac_dvdnr.dylib"));
            RuntimeDependencies.Add(Path.Combine(LibDir, "model.dnr"));
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            string[] Libs = new string[] { Path.Combine(LibDir, "libavutil.so.57"),
                                           Path.Combine(LibDir, "libavcodec.so.59"),
                                           Path.Combine(LibDir, "libavformat.so.59"),
                                           Path.Combine(LibDir, "libdvclient.so"),
                                           Path.Combine(LibDir, "libdolbyio_comms_media.so"),
                                           Path.Combine(LibDir, "libdolbyio_comms_sdk.so") };
            PublicAdditionalLibraries.AddRange(new string[] {
                Path.Combine(LibDir, "libdolbyio_comms_media.so"),
                Path.Combine(LibDir, "libdolbyio_comms_sdk.so"),
            });
            PublicDelayLoadDLLs.AddRange(Libs);

            foreach (string Lib in Libs)
            {
                RuntimeDependencies.Add(Lib);
            }
            RuntimeDependencies.Add(Path.Combine(LibDir, "libcac_dvdnr.so"));
            RuntimeDependencies.Add(Path.Combine(LibDir, "model.dnr"));
        }
    }
}
