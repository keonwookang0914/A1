// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

[SupportedPlatforms(UnrealPlatformClass.Server)]
public class A1ServerTarget : TargetRules
{
	public A1ServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;

		ExtraModuleNames.AddRange(new string[] { "A1Game" });

		A1GameTarget.ApplySharedLyraTargetSettings(this);

		bUseChecksInShipping = true;
	}
}
