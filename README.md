[![Build Unreal Plugin](https://github.com/DolbyIO/comms-sdk-unreal/actions/workflows/build.yml/badge.svg)](https://github.com/DolbyIO/comms-sdk-unreal/actions/workflows/build.yml)
[![Documentation](https://github.com/DolbyIO/comms-sdk-unreal/actions/workflows/deploy-to-github-pages.yml/badge.svg)](https://github.com/DolbyIO/comms-sdk-unreal/actions/workflows/deploy-to-github-pages.yml)
[![License](https://img.shields.io/github/license/DolbyIO/comms-sdk-unreal)](LICENSE)

# Dolby.io Virtual World plugin for Unreal Engine

With the Dolby.io Virtual World plugin for Unreal Engine, you can easily integrate [Dolby.io](https://dolby.io) Spatial Audio, powered by Dolby Atmos technology into your virtual world applications.

You can find the plugin documentation here: [Online documentation](https://api-references.dolby.io/comms-sdk-unreal/)

## Supported platforms

The plugin is compatible with the following operating systems:
- Windows 10+
- macOS 10.14+ if you use UE4
- macOS 12+ if you use UE5

## Prerequisites

Before you start, make sure that you have:
- Unreal Engine 4.27 or 5.0 installed on your computer. For more details, see the [Unreal Engine download](https://www.unrealengine.com/en-US/download) page and the [4.27](https://docs.unrealengine.com/4.27/en-US/Basics/InstallingUnrealEngine/) or [5.0](https://docs.unrealengine.com/5.0/en-US/installing-unreal-engine/) installation manuals.
- A [Dolby.io](https://dolby.io) account. If you do not have an account, [sign up](https://dolby.io/signup) for free.
- A client access token or an app key and an app secret copied from the Communications & Media [Dolby.io Dashboard](https://dashboard.dolby.io/).

## Install the plugin

1. Make sure that your project contains a `Plugins` folder in the root of your game folder. If you do not have this folder, create it.
2. Download the latest plugin [release](https://github.com/DolbyIO/comms-sdk-unreal/releases) for your platform and unpack the plugin to the `Plugins` folder.
3. Launch your project using the Unreal Editor and select `Edit > Plugins` from the menu to enable the plugin manually. This step requires restarting the editor.

## macOS permissions

Using the plugin in the Unreal Editor requires the editor to obtain microphone and camera permissions. However, on macOS the editor never asks for permissions, so you need to forcefully provide them to your application. To do so, you can use the [tccutil](https://github.com/DocSystem/tccutil) permissions manager and the following commands:  
- Epic Games Launcher: `sudo python tccutil.py -e -id com.epicgames.EpicGamesLauncher --microphone --camera`
- Unreal Engine 4: `sudo python tccutil.py -e -id com.epicgames.UE4Editor --microphone --camera`
- Unreal Engine 5: `sudo python tccutil.py -e -id com.epicgames.UnrealEditor --microphone --camera`

The mentioned permissions manager is not endorsed by Dolby in any way and may be dangerous as it needs root permissions to access sensitive system files and requires you to grant full disk access to the terminal. If you do not wish to use it, you need to find another way to provide the required permissions to the Unreal Editor. Otherwise, you need to package the game to use the plugin and cannot test it in the editor. In order to package games using the plugin with the data required to request the necessary permissions, you need to add the following lines to your game's `Info.plist` file, or, if you want to automatically add these lines to all your packaged games, to the `{UnrealEngineRoot}/Engine/Source/Runtime/Launch/Resources/Mac/Info.plist` file:

```
<key>NSMicrophoneUsageDescription</key>
<string>Dolby.io Virtual World</string>
<key>NSCameraUsageDescription</key>
<string>Dolby.io Virtual World</string>
```

We recommend the latter solution if it does not conflict with your setup because the `Info.plist` file is overwritten each time the game is packaged.
