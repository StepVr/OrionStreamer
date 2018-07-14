
using System;
using System.IO;
using UnrealBuildTool;
using System.Security.Cryptography;
using System.Text;

public class OrionVRPN : ModuleRules
{

    private string ModulePath
    {
        get { return ModuleDirectory; }
    }

    private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModulePath, "")); }
    }

    public OrionVRPN(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		if (Target.Platform == UnrealTargetPlatform.Mac )
		{
			return;
		}

		//
		// Start of the configuration section
		//

        
        string TargetPlatform = "";
        string VRPNLibName = "vrpn";
        switch(Target.Platform)
        {
            case UnrealTargetPlatform.Win32:
                TargetPlatform = "Win32/";
                VRPNLibName = VRPNLibName + "32";
                break;
            case UnrealTargetPlatform.Win64:
                TargetPlatform = "x64/";
                break;
        }
		// Path to the VRPN  root directory
        string VRPNRootDir = Path.Combine(ThirdPartyPath, TargetPlatform);
        Console.WriteLine(VRPNRootDir);
		//
		// End of the configuration section
		//

		// Convert \ to / to avoid escaping problems
        VRPNRootDir = VRPNRootDir.Replace('\\', '/');
        string VRPNIncludeDir = VRPNRootDir + "include/";
        string VRPNLibDir = VRPNRootDir + "lib/";

		// VRPN include paths
        PublicIncludePaths.Add(VRPNIncludeDir);

        // VRPN library path
        PublicLibraryPaths.Add(VRPNLibDir);

        // statically link with VRPN lib
        PublicAdditionalLibraries.Add(VRPNLibName + ".lib");
		
    }
}
