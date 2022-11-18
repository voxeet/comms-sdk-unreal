# Dolby.io Communications plugin for Unreal
Plugin integrating [Dolby.io Communications C++ SDK](https://github.com/DolbyIO/comms-sdk-cpp) with the Unreal Engine.

## Supported environments
- Unreal Engine 4.27.2 and 5.0.3
- Windows 10 and macOS 12

Note: If you want to use the plugin on macOS, see our [advice](#macos).

## Prerequisites
- [Dolby.io](https://dolby.io) account - if you do not have an account, you can [sign up](https://dolby.io/signup) for free.
- [Client access token](https://docs.dolby.io/communications-apis/docs/overview-developer-tools#client-access-token) copied from the [Dolby.io dashboard](https://dashboard.dolby.io/). To create the token, log into the dashboard, create an application, and navigate to the API keys section.

## <a name="usage"></a> How to use the sample Blueprints
1. Download the latest plugin [release](https://github.com/DolbyIO/comms-sdk-unreal/releases).
2. Copy the plugin to {UnrealEngineRoot}/Engine/Plugins, so that you have a folder such as C:\Epic Games\UE_4.27\Engine\Plugins\DolbyIO.  
*Alternatively, copy the plugin to {YourGameRoot}/Plugins.*
3. Launch your game project. If you are starting out from scratch, create a game using the First Person template.
4. Enable the plugin.
5. Place the DolbyIoSampleConferenceBP object in the scene.
6. Launch the game.
7. Press C to open the connection menu.
8. Paste your Dolby.io client access token in the appropriate field.
9. Connect.
10. See the [section below](#how) for how the plugin works and how you can use it in practice.

## How to use the sample C++ class
1. Follow steps 1-4 from the [section above](#usage).
2. Place the DolbyIoSampleConference object in the scene.
3. Make sure the object is selected in the World Outliner.
4. Paste your Dolby.io client access token in the appropriate field in the Details window.
5. Launch the game.
6. See DolbyIoSampleConference.cpp for available key bindings.

## <a name="how"></a>How it works
The plugin wraps the [Dolby.io Communications C++ SDK](https://github.com/DolbyIO/comms-sdk-cpp) and provides a subset of the SDK's functionality through its API, which is visible in Unreal Engine. As a user, the information in the ADolbyIoConference header, along with the samples mentioned below, should provide you with everything you need to get started.

- DolbyIoSampleConferenceBP provides a sample Blueprint implementation to interface with the SDK using the API specified in ADolbyIoConference.
- DolbyIoSampleConference.cpp is similar except it is a C++ example.
- DolbyIoSampleConnectMenu provides a sample Blueprint implementation of a GUI using DolbyIoSampleConferenceBP.

These samples serve as example uses of the plugin but are not meant to be used as-is. Users should implement their own Blueprints, possibly basing them on the samples provided, or implement C++ code using ADolbyIoConference.

It is also not difficult to extend plugin functionality by just knowing that:
- ADolbyIoConference is the interface between the Unreal Engine and the SDK
- FSdkAccess is the class where most of the calls to the SDK are made

Therefore, the typical workflow to provide new features is to extend ADolbyIoConference's API and implement the functionality in FSdkAccess.

## Building from source
1. Download the latest release from [Dolby.io Communications C++ SDK releases](https://github.com/DolbyIO/comms-sdk-cpp/releases).
2. Launch your game project. If you are starting out from scratch, create a game using the First Person C++ template.
3. Unzip the Dolby.io Communications C++ SDK release.
4. Copy or link the unzipped sdk-release folder into the DolbyIO/Source/ThirdParty folder.
5. Create a folder named "Plugins" in your game's root folder if it does not exist.
6. Copy or link the DolbyIO folder into the Plugins folder.
7. Regenerate IDE project files.
7. Close Unreal Editor.
8. Build your game in the Development Editor configuration.
9. Open Unreal Editor.
10. Follow steps 5-10 from the [section above](#usage).

## <a name="macos"></a> macOS advice
Using the plugin in Unreal Editor requires the Editor to obtain microphone permissions. However, Unreal Editor will never ask for the appropriate permissions, so we need to forcefully provide them to the application. One method to do so is to use [this tool](https://github.com/DocSystem/tccutil):  
- Unreal Engine 4: `sudo python tccutil.py -e -id com.epicgames.UE4Editor --microphone`  
- Unreal Engine 5: `sudo python tccutil.py -e -id com.epicgames.UnrealEditor --microphone`

Please be aware that this tool is not endorsed by Dolby in any way and may be dangerous as it needs root permissions to access sensitive system files and requires you to grant full disk access to the terminal. If you do not wish to use it, you will need to find another way to provide microphone permissions to the Unreal Editor, otherwise, you will need to package the game to use the plugin and you will be unable to test it in the Editor. In order to package games using the plugin with the data required to request microphone permissions, you will also need to add these lines:
```
<key>NSMicrophoneUsageDescription</key>
<string>Dolby.io Communications</string>
```
in your game's Info.plist or, if you want to automatically add these lines in all packaged games, in {UnrealEngineRoot}/Engine/Source/Runtime/Launch/Resources/Mac/Info.plist. The latter solution is recommended if it does not conflict with your setup, because the Info.plist file is overwritten each time the game is packaged.
