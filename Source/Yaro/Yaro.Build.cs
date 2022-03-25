// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Yaro : ModuleRules
{
	public Yaro(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "UMG" });

		PrivteDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
	}
}
