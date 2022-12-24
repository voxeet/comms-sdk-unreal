// Copyright 2022 Dolby Laboratories

using System.IO;
using UnrealBuildTool;

public class DolbyIoSubsystemModule : ModuleRules
{
	public DolbyIoSubsystemModule(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		CppStandard = CppStandardVersion.Cpp17;
		bEnableExceptions = true;

		PublicDependencyModuleNames.AddRange(
		    new string[] { "Core", "CoreUObject", "Engine", "HTTP", "Json", "Projects" });

		string SdkDir = Path.Combine("$(PluginDir)", "Source", "ThirdParty", "sdk-release");
		PublicIncludePaths.Add(Path.Combine(SdkDir, "include"));
		string LibDir = Path.Combine(SdkDir, "lib");

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicAdditionalLibraries.AddRange(new string[] {
				Path.Combine(LibDir, "dolbyio_comms_media.lib"),
				Path.Combine(LibDir, "dolbyio_comms_sdk.lib"),
			});

			foreach (string Dll in new string[] { "avutil-56.dll", "dvclient.dll", "dolbyio_comms_media.dll",
				                                  "dolbyio_comms_sdk.dll" })
			{
				PublicDelayLoadDLLs.Add(Dll);
				RuntimeDependencies.Add(Path.Combine("$(PluginDir)/Binaries/Win64", Dll),
				                        Path.Combine(SdkDir, "bin", Dll));
			}
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			string[] Libs = new string[] {
				Path.Combine(LibDir, "libdolbyio_comms_media.dylib"),
				Path.Combine(LibDir, "libdolbyio_comms_sdk.dylib"),
				Path.Combine(LibDir, "libdvclient.dylib"),
			};
			PublicAdditionalLibraries.AddRange(Libs);

			foreach (string Lib in Libs)
			{
				RuntimeDependencies.Add(Lib);
			}
		}
	}
}
