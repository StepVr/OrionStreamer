using UnrealBuildTool;
using System;
using System.IO;
public class OrionDataStreamCore : ModuleRules
{
	public OrionDataStreamCore(ReadOnlyTargetRules Target) : base(Target)
    {
	    PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(new string[] { "Projects", "Json", "JsonUtilities"});
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "OrionVRPN" ,"StepVrPlugin" });

        bFasterWithoutUnity = true;

    }
}
