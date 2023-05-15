---
sidebar_position: 9
sidebar_label: Using the Plugin with C++
title: Using the Plugin with C++
---

After enabling the plugin, you need to use the `UDolbyIOSubsystem` class to be able to use the available features in C++. We will show an example using Unreal Engine 5's First Person C++ game template. For simplicity, we assume that the project is named "Foo".

## Step 1 - Add DolbyIO module
Open `Foo/Source/Foo/Foo.build.cs` and add `"DolbyIO"` in `PublicDependencyModuleNames`:
```
PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "DolbyIO", "Engine", "InputCore", "HeadMountedDisplay" });
```

## Step 2 - Edit FooCharacter.h
Open `Foo/Source/Foo/FooCharacter.h` and add these lines somewhere in the `AFooCharacter` class:
```
UFUNCTION()
void OnDolbyIOInitialized();

class UDolbyIOSubsystem* DolbyIOSubsystem;
```

## Step 3 - Edit FooCharacter.cpp
Open `Foo/Source/Foo/FooCharacter.cpp` and replace the `BeginPlay` function definition with:
```
#include "DolbyIOSubsystem.h"

void AFooCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		DolbyIOSubsystem = GameInstance->GetSubsystem<UDolbyIOSubsystem>();
		if (DolbyIOSubsystem)
		{
			DolbyIOSubsystem->OnInitialized.AddDynamic(this, &AFooCharacter::OnDolbyIOInitialized);
			DolbyIOSubsystem->SetToken("paste token here");
		}
	}
}

void AFooCharacter::OnDolbyIOInitialized()
{
	if (DolbyIOSubsystem)
	{
		DolbyIOSubsystem->DemoConference();
	}
}
```

## Step 4 - Configure access credentials
Provide your client access token in the `DolbyIOSubsystem->SetToken...` line.

## Step 5 - Compile and run
You should connect to the demo conference when the game starts.
