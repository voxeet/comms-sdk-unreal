// Copyright 2022 Dolby Laboratories

using UnrealBuildTool;

public class DolbyIoSubsystemModule : ModuleRules
{
	public DolbyIoSubsystemModule(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
		    new string[] { "Core", "CoreUObject", "Engine", "HTTP", "Json", "SdkAccessModule" });
	}
}
