using System.IO;
using UnrealBuildTool;

public class SdkAccessModule : ModuleRules
{
    public SdkAccessModule(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        CppStandard = CppStandardVersion.Cpp17;
        bEnableExceptions = true;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "Engine" });

        var SdkDir = Path.Combine(Directory.GetParent(ModuleDirectory).Parent.FullName, "ThirdParty", "sdk-release");
        var LibDir = Path.Combine(SdkDir, "lib");

        PublicIncludePaths.AddRange(new string[] {
            Path.Combine(SdkDir, "include"),
        });

        string[] Libs = { };

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            Libs = new string[] {
                Path.Combine(LibDir, "dolbyio_comms_media.lib"),
                Path.Combine(LibDir, "dolbyio_comms_sdk.lib"),
            };
            var BinDir = Path.Combine(SdkDir, "bin");
            foreach (var Dll in new string[] {
                Path.Combine(BinDir, "avutil-56.dll"),
                Path.Combine(BinDir, "dvclient.dll"),
                Path.Combine(BinDir, "dolbyio_comms_media.dll"),
                Path.Combine(BinDir, "dolbyio_comms_sdk.dll"),
                })
                RuntimeDependencies.Add(Path.Combine("$(TargetOutputDir)", Path.GetFileName(Dll)), Dll);
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
            Libs = new string[] {
                Path.Combine(LibDir, "libdolbyio_comms_media.dylib"),
                Path.Combine(LibDir, "libdolbyio_comms_sdk.dylib"),
                Path.Combine(LibDir, "libdvclient.dylib"),
            };

        PublicAdditionalLibraries.AddRange(Libs);

        foreach (string Lib in Libs)
            RuntimeDependencies.Add(Lib);
    }
}
