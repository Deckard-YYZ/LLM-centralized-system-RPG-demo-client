// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class TheProject : ModuleRules
{
	public TheProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "InputCore",
			"Sockets",          // ✅ Needed for ISocketSubsystem
			"Networking",        // ✅ Needed for FIPv4Address
			"Json",             // ✅ Required for FJsonObject, FJsonValue, etc.
			"JsonUtilities"     // ✅ Required for FJsonObjectConverter
		});

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
