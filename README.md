# C++ SDK plugin for Unreal Engine
- Plugin integrating Dolby.io CAPI C++ SDK with Unreal Engine versions 4 and 5.  
- This integration demonstrates Dolby.io's spatial audio in a 3D environment.  
- Tested with Unreal Engine versions 4.27.2 and 5.0.3 on Windows 10 and macOS 12.5.

## Integrating with a game
- If on macOS, follow development advice below.
- If you don't have a game, create a game from a First Person or Third Person template using Unreal Editor.
- Copy or link sdk-release folder into Cppsdk folder. Use build 3e737b021ed595d1a6722238a08dc757018f87c0 or later.
- Copy or link Cppsdk folder into {YourGameRoot}/Plugins (create Plugins folder if it does not exist).
- Regenerate IDE project files - see section below for help.
- Close Unreal Editor if opened.
- Build your game in the Development Editor configuration. On Windows, also build the Development configuration.
- Open Unreal Editor and place the plugin's CallBP object in the scene - see section below for help.

## macOS development advice
- Use [this tool](https://github.com/DocSystem/tccutil) to enable microphone permissions in Unreal Editor(s):  
    - UE4 Editor: `sudo python tccutil.py -e -id com.epicgames.UE4Editor --microphone`  
    - UE5 Editor: `sudo python tccutil.py -e -id com.epicgames.UnrealEditor --microphone`  
- Add these lines:  
`<key>NSMicrophoneUsageDescription</key>`    
`<string>Dolby.io voice call</string>`  
in {UnrealEngineRoot}/Engine/Source/Runtime/Launch/Resources/Mac/Info.plist    
This will package games with the data required to request microphone permissions.

## Windows development advice
- [This tool](https://schinagl.priv.at/nt/hardlinkshellext/linkshellextension.html) is useful for creating symbolic links on Windows.

## Regenerating IDE project files
If your game does not contain C++ code (e.g. you created a game from a Blueprint template), you cannot use Unreal Editor to generate project files until you add at least one C++ class from the Editor.

- Windows: 
    - right-click on the {YourGameRoot}\\.uproject file in explorer -> Generate Visual Studio project files or
        - UE4 Editor: File -> Refresh Visual Studio Project
        - UE5 Editor: Tools -> Refresh Visual Studio Project
- macOS:
    - use the script in {UnrealEngineRoot}/Engine/Build/BatchFiles/Mac/Build.sh or
        - UE4 Editor: File -> Refresh Xcode Project
        - UE5 Editor: Tools -> Refresh Xcode Project

You can modify the default IDE to use, e.g. in Unreal Editor preferences.

## Placing plugin objects in game
- UE4 Editor:
    - Content Browser -> View Options -> Show Plugin Content
    - Content Browser -> Cppsdk Content -> drag CallBP to anywhere in the viewport
- UE5 Editor: 
    - Content Drawer -> Settings -> Show Plugin Content
    - Content Drawer -> All -> Plugins -> Dolby.io C++ SDK Content -> drag CallBP to anywhere in the viewport
