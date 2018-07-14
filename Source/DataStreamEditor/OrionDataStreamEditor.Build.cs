using UnrealBuildTool;

public class OrionDataStreamEditor : ModuleRules
{
	public OrionDataStreamEditor(ReadOnlyTargetRules Target) : base(Target)
    {

        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "UnrealEd", "OrionDataStreamCore",  "AnimGraph", "BlueprintGraph" });
        PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "MessageLog"});


        bFasterWithoutUnity = true;
    }
}