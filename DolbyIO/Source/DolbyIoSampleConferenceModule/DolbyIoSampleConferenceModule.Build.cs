using UnrealBuildTool;

public class DolbyIoSampleConferenceModule : ModuleRules
{
    public DolbyIoSampleConferenceModule(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "Slate",
                "DolbyIoConferenceModule",
            }
            );
    }
}
