# Dolby.io Communications plugin for Unreal Engine
Plugin integrating [Dolby.io Communications C++ SDK](https://github.com/DolbyIO/comms-sdk-cpp) with the Unreal Engine.

## Notes
- Tested with Unreal Engine 4.27.2 and 5.0.3.
- Tested on Windows 10 and macOS 12.
- If on macOS, it is recommended to follow the [advice](#macos) below.

## <a name="usage"></a> How to use
1. Download the latest plugin [release](https://github.com/DolbyIO/comms-sdk-unreal/releases).  
*The location above does not yet work as there is no CI for this project yet. Use the build from [Box](https://dolby.box.com/s/dp6seu7ezine09my1tgmlc21k4w7pmmc)*.
2. Copy the plugin to {UnrealEngineRoot}/Engine/Plugins, so that you have a folder such as C:\Epic Games\UE_4.27\Engine\Plugins\DolbyIO.  
*Alternatively, copy the plugin to {YourGameRoot}/Plugins.*
3. Launch your game project ([create a game](#create) using the First Person template if you're starting out from scratch).
4. [Enable](#enable) the plugin.
5. [Place](#place) the plugin's DolbyIoSampleConferenceBP object in the scene.
6. Launch the game.
7. Press C to open the connection menu.
8. Paste your [Dolby.io](https://dolby.io) client access token in the appropriate field.
9. Connect.
10. See [section below](#how) for how the plugin works and how you can use it in practice.

## <a name="create"></a> Creating game from template
Launch Unreal Engine from Epic Games Launcher, then:
- Unreal Engine 4: Games → Next → First Person → Next → choose name and folder → Create Project.
- Unreal Engine 5: Games → First Person → choose name and folder → Create.

You may replace the First Person template with any other template, but Third Person is the only other one that makes sense.

## <a name="enable"></a> Enabling the plugin
- Unreal Engine 4: Edit → Plugins → Installed → Audio → Dolby.io Communications → tick "Enabled" → restart when prompted

## <a name="place"></a> Placing sample plugin object in game
- Unreal Engine 4:
    - Content Browser → View Options → Show Plugin Content + Show Engine Content
    - Content Browser → DolbyIO Content → drag DolbyIoSampleConferenceBP to anywhere in the viewport
- Unreal Engine 5:
    - Content Drawer → Settings → Show Plugin Content + Show Engine Content
    - Content Drawer → All → Plugins → Dolby.io Communications Content → drag DolbyIoSampleConferenceBP to anywhere in the viewport

## <a name="macos"></a> macOS advice
Using the plugin in Unreal Editor requires Unreal Editor to obtain microphone permissions. However, Unreal Editor will never ask for the appropriate permissions, so we need to forcefully provide them to the application. One method to do so is to use [this tool](https://github.com/DocSystem/tccutil):  
- Unreal Engine 4: `sudo python tccutil.py -e -id com.epicgames.UE4Editor --microphone`  
- Unreal Engine 5: `sudo python tccutil.py -e -id com.epicgames.UnrealEditor --microphone`

Please be aware that this tool is not endorsed by Dolby in any way and may be dangerous as it needs root permissions to access sensitive system files. If you do not wish to use it, you will need to find another way to provide microphone permissions to the Unreal Editor, otherwise you will need to package the game to use the plugin and you will be unable to test it in the Editor. In order to package games using the plugin with the data required to request microphone permissions, you will also need to add these lines:
```
<key>NSMicrophoneUsageDescription</key>
<string>Dolby.io Communications</string>
```
in your game's Info.plist or, if you want to automatically add these lines in all packaged games, in {UnrealEngineRoot}/Engine/Source/Runtime/Launch/Resources/Mac/Info.plist. The latter solution is recommended if it does not conflict with your setup, because the Info.plist file is overwritten each time the game is packaged.

## Building from source
1. Download [Dolby.io Communications C++ SDK release 2.1.0](https://github.com/DolbyIO/comms-sdk-cpp/releases/tag/2.1.0).
2. Launch your game project ([create a game](#create) using the First Person template if you're starting out from scratch).
3. Unzip the Dolby.io Communications C++ SDK release.
4. Copy or link the unzipped sdk-release folder into the DolbyIO/ThirdParty folder.
5. Create folder named "Plugins" in your game's root folder if it does not exist.
6. Copy or link the DolbyIO folder into the Plugins folder.
7. [Regenerate](#regenerate) IDE project files.
7. Close Unreal Editor.
8. Build your game in the Development Editor configuration.
9. Open Unreal Editor.
10. Follow steps 5-10 from the [usage](#usage) section.

## <a name="regenerate"></a> Regenerating IDE project files
If your game does not contain C++ code (e.g. you created a game from a Blueprint template), you cannot use Unreal Editor to generate project files until you add at least one C++ class from the Editor.
- Windows: 
    - right-click on the Unreal project file in explorer → Generate Visual Studio project files or
        - Unreal Engine 4: File → Refresh Visual Studio Project
        - Unreal Engine 5: Tools → Refresh Visual Studio Project
- macOS:
    - use the script in {UnrealEngineRoot}/Engine/Build/BatchFiles/Mac/GenerateProjectFiles.sh or
        - Unreal Engine 4: File → Refresh Xcode Project
        - Unreal Engine 5: Tools → Refresh Xcode Project

## <a name="how"></a>How it works
- DolbyIoSampleConferenceBP provides a sample implementation to interface with the Dolby.io Communications C++ SDK using the API specified in the ADolbyIoConference class.
- DolbyIoSampleConnectMenu provides a sample implementation of a graphics user interface using DolbyIoSampleConferenceBP.
- DolbyIoSampleDebugInfo provides a sample implementation of displaying some additional debug information.

These Blueprints serve as example uses of the plugin but are not meant to be used as-is. Users should implement their own Blueprints, possibly basing them on the samples provided, or implement a C++ class deriving from ADolbyIoConference.

The plugin does several things that the user would normally need to do manually when working with the SDK:
- sets up the build system to include and link appropriate files
- initializes the SDK using the client access token
- connects to and disconnects from conferences and keeps track of the connection status
- updates spatial audio information
- manages audio devices and input/output muting
- performs appropriate conversions due to the SDK using STL
- on Windows, passes the Unreal Engine memory allocator for use in the SDK

The plugin provides a subset of the functionality the SDK offers, but its code structure is fairly simple and extending it should be relatively easy. The key actors in the plugin are:
- ADolbyIoConference - the boundary between Unreal Engine and the SDK, interfacing from external code to SdkAccess and back
- SdkAccess - the class where most of the calls to the SDK are made
- SdkStatus - keeps track of state, interfacing from SdkAccess to ADolbyIoConference
