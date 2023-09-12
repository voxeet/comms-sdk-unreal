---
sidebar_position: 12
sidebar_label: macOS Permissions
title: macOS Permissions
---

## Unreal Editor

Using the plugin in the Unreal Editor requires the editor to obtain microphone and camera permissions, however:
- On Unreal Engine 5.0 and earlier, the editor is not packaged correctly to ask for either permission.
- On Unreal Engine 5.1, the editor is not packaged correctly to ask for the camera permission.
- On Unreal Engine 5.2, the editor is packaged correctly to ask for both permissions.
- On Unreal Engine 5.3, the editor is again not packaged correctly to ask for either permission.

Therefore, you may need to forcefully provide these permissions to the editors and possibly the Epic Games Launcher. To do so, you can use the [tccutil](https://github.com/DocSystem/tccutil) permissions manager and the following commands:
- Unreal Engine 4: `sudo python tccutil.py -e -id com.epicgames.UE4Editor --microphone --camera`
- Unreal Engine 5: `sudo python tccutil.py -e -id com.epicgames.UnrealEditor --microphone --camera`
- Epic Games Launcher: `sudo python tccutil.py -e -id com.epicgames.EpicGamesLauncher --microphone --camera`

The mentioned permissions manager is not endorsed by Dolby in any way and may be dangerous as it needs root permissions to access sensitive system files and requires you to grant full disk access to the terminal. If you do not wish to use it, you need to find another way to provide the required permissions to the Unreal Editor. Otherwise, you need to package the game to use the plugin and cannot test it in the editor.

## Packaging games

In order to package games using the plugin with the data required to request the necessary permissions, you need to add the following lines to your game's `Info.plist` file, or, if you want to automatically add these lines to all your packaged games, to the `{UnrealEngineRoot}/Engine/Source/Runtime/Launch/Resources/Mac/Info.plist` file:

```
<key>NSMicrophoneUsageDescription</key>
<string>Dolby.io Virtual Worlds</string>
<key>NSCameraUsageDescription</key>
<string>Dolby.io Virtual Worlds</string>
```

We recommend the latter solution if it does not conflict with your setup because the `Info.plist` file is overwritten each time the game is packaged. Note that Unreal Engine 5.1 and later provide the `NSMicrophoneUsageDescription` entry, so you only need to add the `NSCameraUsageDescription`.
