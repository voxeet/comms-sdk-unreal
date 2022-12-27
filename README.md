# Dolby.io Virtual World plugin for Unreal Engine
Plugin integrating [Dolby.io Communications](https://dolby.io) with the Unreal Engine.

## Supported environments
- Unreal Engine 4.27.2 and 5.0.3
- Windows 10 and macOS 12

Note: If you want to use the plugin on macOS, see our [advice](#macos).

## Prerequisites
- A [Dolby.io](https://dolby.io) account - if you do not have an account, you can [sign up](https://dolby.io/signup) for free.

## <a name="getting_started"></a> Getting started
1. Download the latest plugin [release](https://github.com/DolbyIO/comms-sdk-unreal/releases).
2. Copy the plugin to {UnrealEngineRoot}/Engine/Plugins, so that you have a folder such as C:\Epic Games\UE_4.27\Engine\Plugins\DolbyIO.  
*Alternatively, copy the plugin to {YourGameRoot}/Plugins.*
3. Launch your game project. If you are starting out from scratch, create a game using the First Person template.
4. If the plugin is not enabled automatically, enable it manually in the Edit->Plugins window and restart Unreal Editor.
5. In the Content Browser, navigate to the plugin's C++ Classes and then to the DolbyIOModule folder.
6. Right-click on the DolbyIO class and select "Create Blueprint class based on DolbyIO".
7. In the Event Graph of the newly created Blueprint class, hover over Functions on the left and select Override->"On Token Needed". You will also need to override the "On Initialized" function. Both events should appear in the Blueprint.
8. The "On Token Needed" event is triggered when the game starts and when a refreshed [client access token](https://docs.dolby.io/communications-apis/docs/overview-developer-tools#client-access-token) is needed. In production, when this event is received, you should obtain a token for your Dolby.io application and call this Blueprint's "Set Token" function. For quick testing, you can manually obtain a token from the [Dolby.io dashboard](https://dashboard.dolby.io/) and paste it directly into the "Set Token" node. You can also use the "Set Token Using Key and Secret" function for convenience during onboarding, but never use this function in production and do not allow your app key and secret to leak.
9. The "On Initialized" event is triggered when the plugin is initialized and ready for use. You can now, for example, call this Blueprint's "Connect" function. Once connected, the "On Connected" event will trigger. There are more events, but to get started, you only need to handle the "On Token Needed" and "On Initialized" events.

## Building from source
1. Download and unzip the latest Dolby.io Communications C++ SDK [release](https://github.com/DolbyIO/comms-sdk-cpp/releases).
2. Copy or link the unzipped sdk-release folder into the DolbyIO/Source/ThirdParty folder.
3. Launch your game project. If you are starting out from scratch, create a game using the First Person C++ template.
4. Create a folder named "Plugins" in your game's root folder if it does not exist.
5. Copy or link the DolbyIO folder into the Plugins folder.
6. Regenerate IDE project files.
7. Close Unreal Editor.
8. Build your game in the Development Editor configuration.

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
