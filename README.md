# Dolby.io Virtual World plugin for Unreal Engine
Plugin integrating [Dolby.io Communications](https://dolby.io) with the Unreal Engine.

## Supported environments
- Unreal Engine 4.27.2 and 5.0.3
- Windows 10 and macOS 12

Note: If you want to use the plugin on macOS, see our [advice](#macos-advice).

## Prerequisites
- A [Dolby.io](https://dolby.io) account - if you do not have an account, you can [sign up](https://dolby.io/signup) for free.

## Getting started
1. Download the latest plugin [release](https://github.com/DolbyIO/comms-sdk-unreal/releases).
2. Copy the plugin to {UnrealEngineRoot}/Engine/Plugins, so that you have a folder such as C:\Epic Games\UE_4.27\Engine\Plugins\DolbyIO.  
*Alternatively, copy the plugin to {YourGameRoot}/Plugins.*
3. Launch your game project. If you are starting out from scratch, create a game using the First Person template.
4. If the plugin is not enabled automatically, enable it manually in the Edit->Plugins window and restart Unreal Editor.
5. You need to handle two events generated by the plugin's Dolby.io Subsystem: [On Token Needed](#on-token-needed) and [On Initialized](#on-initialized). There are more [events](#events), but to get started, you only need to handle these two. You have two basic ways to do this. 
    - from any Blueprint:  
Right-click in the Blueprint and add a Dolby.io Subsystem node (Get DolbyIOSubsystem). Drag off the subsystem and type "assign". You will then need to assign delegates for [On Token Needed](#on-token-needed) and [On Initialized](#on-initialized):  
![example](Images/assign_delegate.PNG)
    - from Blueprints which can contain an Actor Component:  
Open the Blueprint, click Add Component, select Dolby.io Observer. When you select the DolbyIOObserver component, a convenient list of implementable events appears in the Details panel - simply click the plus sign next to [On Token Needed](#on-token-needed) and [On Initialized](#on-initialized) and implement the event handlers in the Blueprint:  
![example](Images/add_component.PNG)  
![example](Images/component_events.PNG)
6. Use the Dolby.io Subsystem node (Get DolbyIOSubsystem) to call the plugin [functions](#functions) and handle the [events](#events) it generates.  
![example](Images/calling_functions.PNG)

## Events
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

Example:  
![example](Images/on_connected.PNG)

### On Disconnected
Triggered when the client is disconnected from the conference by any means; in particular, by the [Disconnect](#disconnect) function.

Example:  
![example](Images/on_disconnected.PNG)

### On Participant Added
Triggered when a remote participant is added to the conference. Provides information about the participant in its argument.

Example:  
![example](Images/on_participant_added.PNG)

### On Participant Left
Triggered when a remote participant leaves the conference. Provides information about the participant in its argument.

Example:  
![example](Images/on_participant_left.PNG)

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

You may use the [Set Token Using Key and Secret](#set-token-using-key-and-secret) function instead for convenience during onboarding.

Example:  
![example](Images/on_token_needed.PNG)

### Set Token Using Key and Secret
Initializes or refreshes the client access token. The function is similar to [Set Token](#set-token), except it takes an app key and secret as parameters and automatically generates the token.

For convenience during early development and prototyping, this function is provided to acquire the client access token directly from within the application. However, please note **we do not recommend** using this mechanism in the production software for [security best practices](https://docs.dolby.io/communications-apis/docs/guides-client-authentication). App secret needs to be protected and not included in the application.

Example:  
![example](Images/set_token_using_key_and_secret.PNG)

### Connect
Connects to a conference. The method takes a conference name and user name as parameters. Triggers [On Connected](#on-connected) if successful.

Example:  
![example](Images/on_initialized.PNG)

### Demo Conference
Connects to a demo conference, which automatically brings in 3 invisible bots into the conference as a quick way to validate the connection to the service with audio functionality. One of the bots is placed to the left of point {0, 0, 0} in the level, one is placed to the right and one circles around that point. Triggers [On Connected](#on-connected) if successful.

Example:  
![example](Images/demo_conference.PNG)

### Disconnect
Disconnects from the current conference. Triggers [On Disconnected](#on-disconnected) when complete.

Example:  
![example](Images/disconnect.PNG)

### Set Spatial Environment Scale
Sets the spatial environment scale. The larger the scale, the longer the distance at which the spatial audio attenuates. To get the best experience, the scale should be set separately for each level. The default value of "1.0" means that audio will fall completely silent at a distance of 10000 units (10000 cm/100 m).

Example:  
![example](Images/set_spatial_environment_scale.PNG)

### Mute Input
Mutes audio input.

### Unmute Input
Unmutes audio input.

Example:  
![example](Images/mute_input.PNG)

### Mute Output
Mutes audio output.

### Unmute Output
Unmutes audio output.

Example:  
![example](Images/mute_output.PNG)

### Get Audio Levels
Gets audio levels for all speaking participants. Triggers [On Audio Levels Changed](#on-audio-levels-changed) if successful.

Example:  
![example](Images/get_audio_levels.PNG)

### Update View Point
Updates the position and rotation of the listener for spatial audio purposes. Calling this function even once disables the default behavior, which is to automatically use the location and rotation of the first player controller.

Example:  
![example](Images/update_view_point.PNG)

## Building from source
1. Download and unzip the latest Dolby.io Communications C++ SDK [release](https://github.com/DolbyIO/comms-sdk-cpp/releases).
2. Copy or link the unzipped sdk-release folder into the DolbyIO/Source/ThirdParty folder.
3. Launch your game project. If you are starting out from scratch, create a game using the First Person C++ template.
4. Create a folder named "Plugins" in your game's root folder if it does not exist.
5. Copy or link the DolbyIO folder into the Plugins folder.
6. Regenerate IDE project files.
7. Close Unreal Editor.
8. Build your game in the Development Editor configuration.

## macOS advice
Using the plugin in Unreal Editor requires the Editor to obtain microphone and camera permissions. However, Unreal Editor will never ask for the appropriate permissions, so we need to forcefully provide them to the application. One method to do so is to use [this tool](https://github.com/DocSystem/tccutil):  
- Unreal Engine 4: `sudo python tccutil.py -e -id com.epicgames.UE4Editor --microphone --camera`  
- Unreal Engine 5: `sudo python tccutil.py -e -id com.epicgames.UnrealEditor --microphone --camera`

Please be aware that this tool is not endorsed by Dolby in any way and may be dangerous as it needs root permissions to access sensitive system files and requires you to grant full disk access to the terminal. If you do not wish to use it, you will need to find another way to provide the required permissions to the Unreal Editor, otherwise, you will need to package the game to use the plugin and you will be unable to test it in the Editor. In order to package games using the plugin with the data required to request the necessary permissions, you will also need to add these lines:
```
<key>NSMicrophoneUsageDescription</key>
<string>Dolby.io Communications</string>
<key>NSCameraUsageDescription</key>
<string>Dolby.io Communications</string>
```
in your game's Info.plist or, if you want to automatically add these lines in all packaged games, in {UnrealEngineRoot}/Engine/Source/Runtime/Launch/Resources/Mac/Info.plist. The latter solution is recommended if it does not conflict with your setup because the Info.plist file is overwritten each time the game is packaged.
