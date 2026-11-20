// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class A1ClientTarget : TargetRules
{
	public A1ClientTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Client;

		ExtraModuleNames.AddRange(new string[] { "A1Game" });

		A1GameTarget.ApplySharedLyraTargetSettings(this);
	}
}
