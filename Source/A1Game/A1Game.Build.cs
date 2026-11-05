// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class A1Game : ModuleRules
{
	public A1Game(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
                "A1Game"
            }
		);

		PrivateIncludePaths.AddRange(
			new string[] {
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"GameplayTags",
				"GameplayTasks",
				"GameplayAbilities",
            }
		);

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"InputCore",
				"Slate",
				"SlateCore",
				"EnhancedInput",
				"UMG",
				"CommonUI",
			}
		);

        DynamicallyLoadedModuleNames.AddRange(
			new string[] {
			}
		);

		// Generate compile errors if using DrawDebug functions in test/shipping builds.
		PublicDefinitions.Add("SHIPPING_DRAW_DEBUG_ERROR=1");
		

		SetupGameplayDebuggerSupport(Target);
		SetupIrisSupport(Target);
	}
}
