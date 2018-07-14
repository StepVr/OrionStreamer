using UnrealBuildTool;

public class OrionLiveLink : ModuleRules
{
	public OrionLiveLink(ReadOnlyTargetRules Target) : base(Target)
    {
       // PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "Json", "JsonUtilities" ,
            "Messaging",
            "LiveLinkInterface",
            "LiveLinkMessageBusFramework",
            "LiveLink",
            "UnrealEd", "OrionVRPN",  "AnimGraph", "BlueprintGraph" });
        PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "MessageLog"});

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "UnrealEd",
                "Engine",
                "Projects",

                "WorkspaceMenuStructure",
                "EditorStyle",
                "SlateCore",
                "Slate",
                "InputCore",

                "Messaging",
                "LiveLinkInterface",
                "LiveLinkMessageBusFramework",
                "BlueprintGraph",
                "AnimGraph",
            }
        );
        bFasterWithoutUnity = true;
    }
}