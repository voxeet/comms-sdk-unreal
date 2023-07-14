---
sidebar_position: 13
sidebar_label: Targetting Android
title: Targetting Android
---

This tutorial explains how to set up the plugin to target Android devices.

## Prerequisites
- JDK 17 or later

## Setup
1. Follow the [installation](installation) instructions for your development platform.
2. Make sure that the Unreal Editor is closed.
3. Download a plugin [release](https://github.com/DolbyIO/comms-sdk-unreal/releases) for Android and unpack the plugin to the `Plugins` folder. The release version should match your development platform release. Say yes to any overwrite prompts.
4. Edit `{UnrealEngineRoot}/Build/Android/Java/gradle/gradle/wrapper/gradle-wrapper.properties` and modify `distributionUrl` to `https\://services.gradle.org/distributions/gradle-7.4.1-all.zip`
5. Open your project in Unreal Editor.
6. Set `Project Settings -> Platforms -> Android SDK -> Location of JAVA` to the directory containing the required JDK version.
7. Tick the `Project Settings -> Platforms -> Android -> Advanced Build -> Disable libc++_shared dependency validation in all dependencies` checkbox.
