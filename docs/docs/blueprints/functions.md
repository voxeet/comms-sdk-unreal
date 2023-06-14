# Functions

## Dolby.io Set Token

Initializes or refreshes the client access token. Initializes the plugin unless already initialized.

For quick testing, you can manually obtain a token from the [Dolby.io dashboard](https://dashboard.dolby.io) and paste it directly into the node or use the [Get Dolby.io Token](#get-dolbyio-token) function.

| Name | Direction | Type | Default value | Description |
|---|:---|:---|:---|:---|
| **Token** | Input | string | | The client access token. |

| Triggers event | When |
|---|:---|
| [**On Initialized**](events.md#on-initialized) | Initialization is successful |

---

## Get Dolby.io Token

Gets a Dolby.io client access token.

For convenience during early development and prototyping, this function is provided to acquire an client access token directly from the game.

> ⚠️ Using this function effectively distributes the permanent app credential with your Unreal application, which is not safe for production deployment. Follow our [security best practices](https://docs.dolby.io/communications-apis/docs/guides-client-authentication) to set up a server through which you can acquire a temporary client access token.

| Name | Direction | Type | Default value | Description |
|---|:---|:---|:---|:---|
| **App Key** | Input | string | | The app key. |
| **App Secret** | Input | string | | The app secret. |
| **Token Expiration Time In Seconds** | Input | integer | 3600 (1 hour) | The token's expiration time (in seconds). |

| Triggers event | When |
|---|:---|
| [**On Token Obtained**](events.md#on-token-obtained) | Successful |

---

## Dolby.io Demo Conference

Connects to a demo conference.

The demo automatically brings in 3 invisible bots into the conference as a quick way to validate the connection to the service with audio functionality. The bots are placed at point {0, 0, 0}.

| Name | Direction | Type | Default value | Description |
|---|:---|:---|:---|:---|
| **N/A** | | | | |

| Triggers event | When |
|---|:---|
| [**On Connected**](events.md#on-connected) | Successful |

---

## Dolby.io Connect

Connects to a conference.

| Name | Direction | Type | Default value | Description |
|---|:---|:---|:---|:---|
| **Conference Name** | Input | string | "unreal" | The conference name. Must not be empty. |
| **User Name** | Input | string | "" | The name of the participant. |
| **External ID** | Input | string | "" | The external unique identifier that the customer's application can add to the participant while opening a session. If a participant uses the same external ID in conferences, the participant's ID also remains the same across all sessions. |
| **Avatar URL** | Input | string | "" | The URL of the participant's avatar. |
| **Connection Mode** | Input | [Dolby.io Connection Mode](types.md#dolbyio-connection-mode) | Active | Defines whether to connect as an active user or a listener. |
| **Spatial Audio Style** | Input | [Dolby.io Spatial Audio Style](types.md#dolbyio-spatial-audio-style) | Shared | The spatial audio style of the conference. |
| **Max Video Streams** | Input | integer | 25 | Sets the maximum number of video streams that may be transmitted to the user. Valid parameter values are between 0 and 25. |
| **Video Forwarding Strategy** | Input | [Dolby.io Video Forwarding Strategy](types.md#dolbyio-video-forwarding-strategy) | LastSpeaker | Defines how the plugin should select conference participants whose videos will be transmitted to the local participant. |

| Triggers event | When |
|---|:---|
| [**On Connected**](events.md#on-connected) | Successful |

---

## Dolby.io Disconnect

Disconnects from the current conference.

| Name | Direction | Type | Default value | Description |
|---|:---|:---|:---|:---|
| **N/A** | | | | |

| Triggers event | When |
|---|:---|
| [**On Disconnected**](events.md#on-connected) | Successful |

---

## Dolby.io Set Spatial Environment Scale

Sets the spatial environment scale.

The larger the scale, the longer the distance at which the spatial audio attenuates. To get the best experience, the scale should be set separately for each level. The default value of "1.0" means that audio will fall completely silent at a distance of 10000 units (10000 cm/100 m).

| Name | Direction | Type | Default value | Description |
|---|:---|:---|:---|:---|
| **Scale** | Input | float | 1.0 | The scale as a floating point number. |

| Triggers event | When |
|---|:---|
| **N/A** | |

---

## Dolby.io Mute Input

Mutes audio input.

| Name | Direction | Type | Default value | Description |
|---|:---|:---|:---|:---|
| **N/A** | | | | |

| Triggers event | When |
|---|:---|
| **N/A** | |

---

## Dolby.io Unmute Input

Unmutes audio input.

| Name | Direction | Type | Default value | Description |
|---|:---|:---|:---|:---|
| **N/A** | | | | |

| Triggers event | When |
|---|:---|
| **N/A** | |

---

## Dolby.io Mute Output

Mutes audio output.

| Name | Direction | Type | Default value | Description |
|---|:---|:---|:---|:---|
| **N/A** | | | | |

| Triggers event | When |
|---|:---|
| **N/A** | |

---

## Dolby.io Unmute Output

Unmutes audio output.

| Name | Direction | Type | Default value | Description |
|---|:---|:---|:---|:---|
| **N/A** | | | | |

| Triggers event | When |
|---|:---|
| **N/A** | |

---

## Dolby.io Mute Participant

Mutes a given participant for the local user.

| Name | Direction | Type | Default value | Description |
|---|:---|:---|:---|:---|
| **Participant ID** | Input | string | | The ID of the remote participant to mute. |

| Triggers event | When |
|---|:---|
| **N/A** | |

---

## Dolby.io Unmute Participant

Unmutes a given participant for the local user.

| Name | Direction | Type | Default value | Description |
|---|:---|:---|:---|:---|
| **Participant ID** | Input | string | | The ID of the remote participant to unmute. |

| Triggers event | When |
|---|:---|
| **N/A** | |

---

## Dolby.io Get Participants

Gets a list of all remote participants.

| Name | Direction | Type | Default value | Description |
|---|:---|:---|:---|:---|
| **Return Value** | Output | array of [Dolby.io Participant Info](types.md#dolbyio-participant-info) | | An array of current Dolby.io Participant Info's. |

| Triggers event | When |
|---|:---|
| **N/A** | |

---

## Dolby.io Enable Video

Enables video streaming from the given video device or the default device if no device is given.

| Name | Direction | Type | Default value | Description |
|---|:---|:---|:---|:---|
| **VideoDevice** | Input | [Dolby.io Video Device](types.md#dolbyio-video-device) | default device | The video device to use. |

| Triggers event | When |
|---|:---|
| [**On Video Enabled**](events.md#on-video-enabled) | Successful |

---

## Dolby.io Disable Video

Disables video streaming.

| Name | Direction | Type | Default value | Description |
|---|:---|:---|:---|:---|
| **N/A** | | | | |

| Triggers event | When |
|---|:---|
| [**On Video Disabled**](events.md#on-video-disabled) | Successful |

---

## Dolby.io Bind Material

Binds a dynamic material instance to hold the frames of the given video track. The plugin will update the material's texture parameter named "DolbyIO Frame" with the necessary data, therefore the material should have such a parameter to be usable. Automatically unbinds the material from all other tracks, but it is possible to bind multiple materials to the same track. Has no effect if the track does not exist at the moment the function is called, therefore it should usually be called as a response to the [On Video Track Added](events.md#on-video-track-added) event.

| Name | Direction | Type | Default value | Description |
|---|:---|:---|:---|:---|
| **Material** | Input | [Dynamic Material Instance](https://docs.unrealengine.com/5.2/en-US/BlueprintAPI/Rendering/Material/CreateDynamicMaterialInstance/) | | The dynamic material instance to bind. |
| **Video Track ID** | Input | string | | The ID of the video track. |

| Triggers event | When |
|---|:---|
| **N/A** | |

---

## Dolby.io Unbind Material

Unbinds a dynamic material instance to no longer hold the video frames of the given video track. The plugin will no longer update the material's texture parameter named "DolbyIO Frame" with the necessary data.

| Name | Direction | Type | Default value | Description |
|---|:---|:---|:---|:---|
| **Material** | Input | [Dynamic Material Instance](https://docs.unrealengine.com/5.2/en-US/BlueprintAPI/Rendering/Material/CreateDynamicMaterialInstance/) | | The dynamic material instance to unbind. |
| **Video Track ID** | Input | string | | The ID of the video track. |

| Triggers event | When |
|---|:---|
| **N/A** | |

---

## Dolby.io Get Texture

Gets the texture to which video from a given track is being rendered.

| Name | Direction | Type | Default value | Description |
|---|:---|:---|:---|:---|
| **Video Track ID** | Input | string | | The ID of the video track. |
| **Return Value** | Output | [Texture](https://docs.unrealengine.com/5.2/en-US/BlueprintAPI/Rendering/Texture/) | | The texture holding the video track's frame or NULL if no such texture exists. |

| Triggers event | When |
|---|:---|
| **N/A** | |

---

## Dolby.io Get Screenshare Sources

Gets a list of all possible screen sharing sources. These can be entire screens or specific application windows.

| Name | Direction | Type | Default value | Description |
|---|:---|:---|:---|:---|
| **N/A** | | | | |

| Triggers event | When |
|---|:---|
| [**On Screenshare Sources Received**](events.md#on-screenshare-sources-received) | Successful |

---

## Dolby.io Start Screenshare

Starts screen sharing using a given source.

Users should make use of the parameters to optimize for the content they are sharing. For example, for sharing dynamic content like a YouTube video, the ideal settings are MaxResolution=DownscaleTo1080p, EncoderHint=Fluid, DownscaleQuality=High.

| Name | Direction | Type | Default value | Description |
|---|:---|:---|:---|:---|
| **Source** | Input | [Dolby.io Screenshare Source](types.md#dolbyio-screenshare-source) | | The source to use. |
| **Encoder Hint** | Input | [Dolby.io Screenshare Encoder Hint](types.md#dolbyio-screenshare-encoder-hint) | Detailed | Provides a hint to the plugin as to what type of content is being captured by the screen share. |
| **Max Resolution** | Input | [Dolby.io Screenshare Max Resolution](types.md#dolbyio-screenshare-max-resolution) | Actual Captured | The maximum resolution for the capture screen content to be shared as. |
| **Downscale Quality** | Input | [Dolby.io Screenshare Downscale Quality](types.md#dolbyio-screenshare-downscale-quality) | Low | The quality for the downscaling algorithm to be used. |

| Triggers event | When |
|---|:---|
| [**On Screenshare Started**](events.md#on-screenshare-started) | Successful |

---

## Dolby.io Stop Screenshare

Stops screen sharing.

| Name | Direction | Type | Default value | Description |
|---|:---|:---|:---|:---|
| **N/A** | | | | |

| Triggers event | When |
|---|:---|
| [**On Screenshare Stopped**](events.md#on-screenshare-stopped) | Successful |

---

## Dolby.io Change Screenshare Parameters

Changes the screen sharing parameters if already sharing screen.

| Name | Direction | Type | Default value | Description |
|---|:---|:---|:---|:---|
| **Encoder Hint** | Input | [Dolby.io Screenshare Encoder Hint](types.md#dolbyio-screenshare-encoder-hint) | Detailed | Provides a hint to the plugin as to what type of content is being captured by the screen share. |
| **Max Resolution** | Input | [Dolby.io Screenshare Max Resolution](types.md#dolbyio-screenshare-max-resolution) | Actual Captured | The maximum resolution for the capture screen content to be shared as. |
| **Downscale Quality** | Input | [Dolby.io Screenshare Downscale Quality](types.md#dolbyio-screenshare-downscale-quality) | Low | The quality for the downscaling algorithm to be used. |

| Triggers event | When |
|---|:---|
| **N/A** | |

---

## Dolby.io Set Local Player Location

Updates the location of the listener for spatial audio purposes.

Calling this function even once disables the default behavior, which is to automatically use the location of the first player controller.

| Name | Direction | Type | Default value | Description |
|---|:---|:---|:---|:---|
| **Location** | Input | [Vector](https://docs.unrealengine.com/5.2/en-US/BlueprintAPI/Math/Vector/) | | The location of the listener. |

| Triggers event | When |
|---|:---|
| **N/A** | |

---

## Dolby.io Set Local Player Rotation

Updates the rotation of the listener for spatial audio purposes.

Calling this function even once disables the default behavior, which is to automatically use the rotation of the first player controller.

| Name | Direction | Type | Default value | Description |
|---|:---|:---|:---|:---|
| **Rotation** | Input | [Rotator](https://docs.unrealengine.com/5.2/en-US/BlueprintAPI/Math/Rotator/) | | The rotation of the listener. |

| Triggers event | When |
|---|:---|
| **N/A** | |

---

## Dolby.io Set Remote Player Location

Updates the location of the given remote participant for spatial audio purposes.

This is only applicable when the spatial audio style of the conference is set to "Individual".

Calling this function with the local participant ID has no effect. Use [Set Local Player Location](#dolbyio-set-local-player-rotation) instead.

| Name | Direction | Type | Default value | Description |
|---|:---|:---|:---|:---|
| **Participant ID** | Input | string | | The ID of the remote participant. |
| **Location** | Input | [Vector](https://docs.unrealengine.com/5.2/en-US/BlueprintAPI/Math/Vector/) | | The location of the remote participant. |

| Triggers event | When |
|---|:---|
| **N/A** | |

---

## Dolby.io Set Log Settings

Sets what to log in the Dolby.io C++ SDK. The logs will be saved to the default project log directory (likely Saved/Logs).

This function should be called before the first call to [Set Token](#dolbyio-set-token) if the user needs logs about the plugin's operation. Calling this function more than once has no effect.

| Name | Direction | Type | Default value | Description |
|---|:---|:---|:---|:---|
| **Sdk Log Level** | Input | [Dolby.io Log Level](types.md#dolbyio-log-level) | Info | Log level for SDK logs. |
| **Media Log Level** | Input | [Dolby.io Log Level](types.md#dolbyio-log-level) | Info | Log level for Media Engine logs. |
| **Dvc Log Level** | Input | [Dolby.io Log Level](types.md#dolbyio-log-level) | Info | Log level for DVC logs. |

| Triggers event | When |
|---|:---|
| **N/A** | |

---

## Dolby.io Get Audio Input Devices

Gets a list of all available audio input devices.

| Name | Direction | Type | Default value | Description |
|---|:---|:---|:---|:---|
| **N/A** | | | | |

| Triggers event | When |
|---|:---|
| [**On Audio Input Devices Received**](events.md#on-audio-input-devices-received) | Successful |

---

## Dolby.io Get Audio Output Devices

Gets a list of all available audio output devices.

| Name | Direction | Type | Default value | Description |
|---|:---|:---|:---|:---|
| **N/A** | | | | |

| Triggers event | When |
|---|:---|
| [**On Audio Output Devices Received**](events.md#on-audio-output-devices-received) | Successful |

---

## Dolby.io Get Current Audio Input Device

Gets the current audio input device.

| Name | Direction | Type | Default value | Description |
|---|:---|:---|:---|:---|
| **N/A** | | | | |

| Triggers event | When |
|---|:---|
| [**On Current Audio Input Device Received**](events.md#on-current-audio-input-device-received) | Successful |

---

## Dolby.io Get Current Audio Output Device

Gets the current audio output device.

| Name | Direction | Type | Default value | Description |
|---|:---|:---|:---|:---|
| **N/A** | | | | |

| Triggers event | When |
|---|:---|
| [**On Current Audio Output Device Received**](events.md#on-current-audio-output-device-received) | Successful |

---

## Dolby.io Set Audio Input Device

Sets the audio input device.

| Name | Direction | Type | Default value | Description |
|---|:---|:---|:---|:---|
| **Native ID** | Input | string | | The ID of the device to use. |

| Triggers event | When |
|---|:---|
| [**On Current Audio Input Device Changed**](events.md#on-current-audio-input-device-changed) | Successful |

---

## Dolby.io Set Audio Output Device

Sets the audio output device.

| Name | Direction | Type | Default value | Description |
|---|:---|:---|:---|:---|
| **Native ID** | Input | string | | The ID of the device to use. |

| Triggers event | When |
|---|:---|
| [**On Current Audio Output Device Changed**](events.md#on-current-audio-output-device-changed) | Successful |

---

## Dolby.io Get Video Devices

Gets a list of all available video devices.

| Name | Direction | Type | Default value | Description |
|---|:---|:---|:---|:---|
| **N/A** | | | | |

| Triggers event | When |
|---|:---|
| [**On Video Devices Received**](events.md#on-video-devices-received) | Successful |

---
