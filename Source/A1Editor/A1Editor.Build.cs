// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class A1Editor : ModuleRules
{
	public A1Editor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"UnrealEd",
			}
		);
	}
}
