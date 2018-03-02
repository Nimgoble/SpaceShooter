// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SpaceShooter : ModuleRules
{
	public SpaceShooter(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		//PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });
		PublicDependencyModuleNames.AddRange
		(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Networking",
				"Sockets",
				"OnlineSubsystem",
				"OnlineSubsystemUtils",
				"AssetRegistry",
				"AIModule",
				"InputCore",
				"UMG",
				"HTTP",
				"Json"
			}
		);

		PrivateDependencyModuleNames.AddRange
		(
			new string[]
			{
				"CoreUObject",
				"InputCore",
				"Networking",
				"Sockets",
				"OnlineSubsystem",
				"OnlineSubsystemUtils",
				"Slate",
				"SlateCore",
				"GameplayDebugger",
				"HTTP",
				"Json",
			}
		);

		DynamicallyLoadedModuleNames.Add("OnlineSubsystemNull");
	}
}
