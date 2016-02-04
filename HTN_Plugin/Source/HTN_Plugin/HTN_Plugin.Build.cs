using UnrealBuildTool;

public class HTN_Plugin : ModuleRules
{
	public HTN_Plugin(TargetInfo Target)
	{
		
		PublicIncludePaths.AddRange(
			new string[] {
				"HTN_Plugin/Public"
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"HTN_Plugin/Private",
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject", "Engine", "Slate", "SlateCore", "AIModule"
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);

        // if 1, we collect stats on our runtime performance (time it takes to find plan etc.) and print it to log
        Definitions.Add("HTN_LOG_RUNTIME_STATS=1");
        // if 1, we want to compile code that is compatible with the UE4 version that Unreal Tournament uses
        Definitions.Add("HTN_COMPILE_UT_COMPATIBLE=0");
	}
}
