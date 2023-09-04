---
sidebar_position: 13
sidebar_label: Targetting Android
title: Targetting Android
---

This tutorial explains how to set up the plugin to target Android devices.

## Prerequisites

Please follow the [official Android setup guide](https://docs.unrealengine.com/5.3/en-US/how-to-set-up-android-sdk-and-ndk-for-your-unreal-engine-development-environment/) for the Unreal Engine and make sure you can package and launch an Android game on a phone without using the plugin first. The Android version of the plugin supports Unreal Engine 5.3 or later.

## Setup
1. Follow the [installation](installation) instructions for your development platform.
2. Make sure that the Unreal Editor is closed.
3. Download a plugin [release](https://github.com/DolbyIO/comms-sdk-unreal/releases) for Android and unpack the plugin to the `Plugins` folder. The release version should match your development platform release. Say yes to any overwrite prompts.
4. Open your project in Unreal Editor.
5. Tick the `Project Settings -> Platforms -> Android -> Advanced Build -> Disable libc++_shared dependency validation in all dependencies` checkbox.

## UnrealBuildTool

It is also required to rebuild UnrealBuildTool to disable static libc++ linkage on Android.

1. Go to `{UnrealEngineRoot}/Engine/Source/Programs/UnrealBuildTool``.
2. Open the `UnrealBuildTool`` solution in Visual Studio.
3. Open the `Platform/Android/AndroidToolChain.cs` file.
4. Find the line `Result += " -static-libstdc++";` and comment it out or simply delete it.
5. Select the `Development` build configuration (default is probably `Debug`) and build the solution.
