# Dolby.io Virtual World plugin for Unreal Engine
Plugin integrating [Dolby.io Communications](https://dolby.io) with the Unreal Engine.

## Supported platforms
The plugin is compatible with the following Unreal Engine versions:
- 4.27.2
- 5.0.3

The plugin is compatible with the following operating systems:
- Windows 10 or newer
- macOS:
    - UE4: latest Mojave or newer
    - UE5: latest Monterey or newer
    - please see our [advice for macOS](#macos-advice)

## Prerequisites
- Unreal Engine is installed, for more details please reffer to [Unreal Engine download page](https://www.unrealengine.com/en-US/download) and instalation manuals ([4.27](https://docs.unrealengine.com/4.27/en-US/Basics/InstallingUnrealEngine/) or [5.0](https://docs.unrealengine.com/5.0/en-US/installing-unreal-engine/))
- You should have Epic Games account
- A [Dolby.io](https://dolby.io) account - if you do not have an account, you can [sign up](https://dolby.io/signup) for free.
- An Unreal project. Please reffer to [A project](#a-project) section. 

### A project
If you have your own project you would like to use, please do that.
In case you you would like to just evaluate the plugin, feel free to use First Person Shooter template. Please follow proper manual:
- [manual for Unreal Engine 4.27](https://docs.unrealengine.com/4.27/en-US/ProgrammingAndScripting/ProgrammingWithCPP/CPPTutorials/FirstPersonShooter/)
- or [manual for Unreal Engine 5.0](https://docs.unrealengine.com/5.0/en-US/first-person-template-in-unreal-engine/)

## Getting started

### Enabling the plugin in the project
First thing you need to do is enable the pluging for your project. You can use the plugin from [GitHub](https://github.com/DolbyIO/comms-sdk-unreal/releases) repo or from the store. Please reffer to proper section.
After installing the plugin please make sure [permission for microphone and camera](#Permissions) are provided.

#### The store
The section will updated after release the plugin in the store

#### GitHub, using provided binary
1. Ensure your project contains Plugins folder ({YourGameRoot}/Plugins). Create the folder if it does not exist.
2. Download the latest plugin [release](https://github.com/DolbyIO/comms-sdk-unreal/releases) for your platform and unpack to {YourGameRoot}/Plugins folder.
3. Launch your project using the Unreal Editor.
4. Enable the plugin. If the plugin is not enabled automatically, enable it manually in the Edit->Plugins window, you will be asked for restarting Unreal Editor, please do it.
5. After Unreal Editor restart you should be ready to [use the plugin in your project](#using-the-plugin)

#### GitHub, building the plugin from source
1. Ensure your project contains Plugins folder ({YourGameRoot}/Plugins). Create the folder if it does not exist.
2. Get DolbyIO folder from [GitHub](https://github.com/DolbyIO/comms-sdk-unreal/tree/main/DolbyIO) and put it in {YourGameRoot}/Plugins folder.
    - There should be *{YourGameRoot}/Plugins/DolbyIO* folder.
3. Download and unzip [this](https://github.com/DolbyIO/comms-sdk-cpp/releases/tag/2.3.0-beta.2) Dolby.io Communications C++ SDK release.
    - Please select proper binary depending on the platform you are using.
    - Note: Unpacked macOS versions should contain sdk-release-x86 and sdk-release-arm subfolders while unpacked Windows version is proper sdk-release folder.
4. Place the unzipped sdk-release folder inside the *{YourGameRoot}/Plugins/DolbyIO* folder.
- Please use sdk-release-x86 for macOS on Intel and sdk-release-arm for macOS on M1 or M2
5. Regenerate project files.
6. Close Unreal Editor if open.
7. Build the project in the Development Editor configuration.


### Permissions
Using the plugin in Unreal Editor requires the Editor to obtain microphone and camera permissions. Please reffer to proper section based on the platform you are using.

#### Windows platform
There are no specific steps for providing permissions. You can freely use microphone and camera by the plugin.

#### macOS platform
Using the plugin in Unreal Editor requires the Editor to obtain microphone and camera permissions. However, Unreal Editor will never ask for the appropriate permissions, so we need to forcefully provide them to the application. One method to do so is to use [this tool](https://github.com/DocSystem/tccutil):  
- Unreal Engine 4: `sudo python tccutil.py -e -id com.epicgames.UE4Editor --microphone --camera`  
- Unreal Engine 5: `sudo python tccutil.py -e -id com.epicgames.UnrealEditor --microphone --camera`

Please be aware that this tool is not endorsed by Dolby in any way and may be dangerous as it needs root permissions to access sensitive system files and requires you to grant full disk access to the terminal. If you do not wish to use it, you will need to find another way to provide the required permissions to the Unreal Editor, otherwise, you will need to package the game to use the plugin and you will be unable to test it in the Editor. In order to package games using the plugin with the data required to request the necessary permissions, you will also need to add these lines:
```
<key>NSMicrophoneUsageDescription</key>
<string>Dolby.io Virtual World</string>
<key>NSCameraUsageDescription</key>
<string>Dolby.io Virtual World</string>
```
in your game's Info.plist or, if you want to automatically add these lines in all packaged games, in {UnrealEngineRoot}/Engine/Source/Runtime/Launch/Resources/Mac/Info.plist. The latter solution is recommended if it does not conflict with your setup because the Info.plist file is overwritten each time the game is packaged.

### Using the plugin
You should be ready now for using the plugin. The plugin provides *Dolby.io Subsystem* node. The node let you to deal with fetaures provided by the plugin. More details can be found below.

1. You need to handle two events generated by the plugin's Dolby.io Subsystem: [On Token Needed](#on-token-needed) and [On Initialized](#on-initialized). There are more [events](#events), but to get started, you only need to handle these two. You have two basic ways to do this: 
    - From any Blueprint:  
Right-click in the Blueprint and add a Dolby.io Subsystem node (Get DolbyIOSubsystem). Drag off the subsystem and type "assign". You will then need to assign delegates for [On Token Needed](#on-token-needed) and [On Initialized](#on-initialized):  
![example](Images/assign_delegate.PNG)
    - From Blueprints which can contain an Actor Component:  
Open the Blueprint, click Add Component, and select Dolby.io Observer. When you select the DolbyIOObserver component, a convenient list of implementable events appears in the Details panel. Click the plus sign next to [On Token Needed](#on-token-needed) and [On Initialized](#on-initialized), and implement the event handlers in the Blueprint:  
![example](Images/add_component.PNG) ![example](Images/component_events.PNG)
2. Use the Dolby.io Subsystem node (Get DolbyIOSubsystem) to call the plugin [functions](#functions) and handle the [events](#events) it generates.

## Events
Below sections desribe events provided by *Dolby.io Subsystem* node.
### On Token Needed
Triggered when an initial or refreshed [client access token](https://docs.dolby.io/communications-apis/docs/overview-developer-tools#client-access-token) is needed, which happens when the game starts or when a refresh token is requested.

After receiving this event, obtain a token for your Dolby.io application and call the [Set Token](#set-token) function.

Example:  
![example](Images/on_token_needed.PNG)

### On Initialized
Triggered when the plugin is successfully initialized after calling the [Set Token](#set-token) function.

After receiving this event, the plugin is ready for use. You can now, for example, call this Blueprint's [Connect](#connect) function. Once connected, the [On Connected](#on-connected) event will trigger.

Example:  
![example](Images/on_initialized.PNG)

### On Connected
Triggered when the client is successfully connected to the conference after calling the [Connect](#connect) function. The event provides the ID of the local participant in its argument.

### On Disconnected
Triggered when the client is disconnected from the conference by any means; in particular, by the [Disconnect](#disconnect) function.

### On Participant Added
Triggered when a remote participant is added to the conference. Provides information about the participant in its argument.

Example:  
![example](Images/on_participant_added.PNG)

### On Participant Left
Triggered when a remote participant leaves the conference. Provides the participant ID in its argument.

### On Video Track Added
Triggered when a video track is added. Provides the participant ID in its argument.

### On Video Track Removed
Triggered when a video track is removed. Provides the participant ID in its argument.

### On Active Speakers Changed
Triggered when participants start or stop speaking. The event provides the IDs of the current speakers in its argument.

Example:  
![example](Images/on_active_speakers_changed.PNG)

### On Audio Levels Changed
Triggered when there are new audio levels available after calling the [Get Audio Levels](#get-audio-levels) function. The event provides two arrays: an array of IDs of the current speakers and an array of floating point numbers representing each participant's audio level. The order of levels corresponds to the order of ActiveSpeakers. A value of 0.0 represents silence and a value of 1.0 represents the maximum volume.

Example:  
![example](Images/on_audio_levels_changed.PNG)

## Functions
### Set Token
Initializes or refreshes the client access token. The function takes the token as a parameter and initializes the plugin unless already initialized. Successful initialization triggers the [On Initialized](#on-initialized) event.

For quick testing, you can manually obtain a token from the [Dolby.io dashboard](https://dashboard.dolby.io/) and paste it directly into the node.

Example:  
![example](Images/on_token_needed.PNG)

### Get Dolby.io Token
For convenience during early development and prototyping, this function is provided to acquire the client access token directly from within the application. However, please note **we do not recommend** using this mechanism in the production software for [security best practices](https://docs.dolby.io/communications-apis/docs/guides-client-authentication). App secret needs to be protected and not included in the application.

Example:  
![example](Images/get_dolbyio_token.PNG)

### Connect
Connects to a conference. The method takes a conference name and user name as parameters. Triggers [On Connected](#on-connected) if successful.

Example:  
![example](Images/on_initialized.PNG)

### Demo Conference
Connects to a demo conference, which automatically brings in 3 invisible bots into the conference as a quick way to validate the connection to the service with audio functionality. The bots are placed at point {0, 0, 0}. Triggers [On Connected](#on-connected) if successful.

Example:  
![example](Images/demo_conference.PNG)

### Disconnect
Disconnects from the current conference. Triggers [On Disconnected](#on-disconnected) when complete.

### Set Spatial Environment Scale
Sets the spatial environment scale. The larger the scale, the longer the distance at which the spatial audio attenuates. To get the best experience, the scale should be set separately for each level. The default value of "1.0" means that audio will fall completely silent at a distance of 10000 units (10000 cm/100 m).

### Mute Input
Mutes audio input.

### Unmute Input
Unmutes audio input.

### Mute Output
Mutes audio output.

### Unmute Output
Unmutes audio output.

### Enable Video
Enables video streaming from the primary webcam.

### Disable Video
Disables video streaming from the primary webcam.

### Get Texture
Gets the texture to which video from a given participant is being rendered or returns NULL if no such texture exists.

Example:  
![example](Images/get_texture.PNG)

### Set Local Player Location
Updates the location of the listener for spatial audio purposes. Calling this function even once disables the default behavior, which is to automatically use the location of the first player controller.

### Set Local Player Rotation
Updates the rotation of the listener for spatial audio purposes. Calling this function even once disables the default behavior, which is to automatically use the rotation of the first player controller.
