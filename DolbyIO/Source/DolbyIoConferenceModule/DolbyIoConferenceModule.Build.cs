// Copyright 2022 Dolby Laboratories

using UnrealBuildTool;

public class DolbyIoConferenceModule : ModuleRules
{
    public DolbyIoConferenceModule(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "SdkAccessModule",
            }
            );
    }
}
