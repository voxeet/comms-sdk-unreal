# Events

## On Token Needed

| Triggered by | When |
|---|:---|
| **Automatically** | An initial or refreshed [client access token](https://docs.dolby.io/communications-apis/docs/overview-developer-tools#client-access-token) is needed, which happens when the Dolby.io Subsystem is initialized or when a refresh token is requested. After receiving this event, obtain a token for your Dolby.io application and call the Dolby.io Set Token function. |

| Provides | Type | Description |
|---|:---|:---|
| **N/A** | | |

---

## On Initialized

| Triggered by | When |
|---|:---|
| [**Dolby.io Set Token**](functions.md#dolbyio-set-token) | The plugin is successfully initialized. After receiving this event, the plugin is ready for use. You can now, for example, call the [Dolby.io Connect](functions.md#dolbyio-connect) function. |

| Provides | Type | Description |
|---|:---|:---|
| **N/A** | | |

---

## On Connected

| Triggered by | When |
|---|:---|
| [**Dolby.io Connect**](functions.md#dolbyio-connect) | The client is successfully connected to the conference. |
| [**Dolby.io Demo Conference**](functions.md#dolbyio-demo-conference) | The client is successfully connected to the conference. |

| Provides | Type | Description |
|---|:---|:---|
| **Local Participant ID** | string | The ID of the local participant. |
| **Conference ID** | string | The ID of the conference. |

---

## On Disconnected

| Triggered by | When |
|---|:---|
| [**Dolby.io Disconnect**](functions.md#dolbyio-disconnect) | The client is disconnected from the conference. |
| **Automatically** | An error occured that disconnected the client from the conference. |

| Provides | Type | Description |
|---|:---|:---|
| **N/A** | | |

---

## On Participant Added

| Triggered by | When |
|---|:---|
| **Automatically** | A remote participant is added to the conference. |

| Provides | Type | Description |
|---|:---|:---|
| **Status** | [Dolby.io Participant Status](types.md#dolbyio-participant-status) | The status of the participant. |
| **Participant Info** | [Dolby.io Participant Info](types.md#dolbyio-participant-info) | Information about the participant. |

---

## On Participant Updated

| Triggered by | When |
|---|:---|
| **Automatically** | A remote participant's status is updated. Please refer to the [graph of possible status changes](../../static/img/participant-status-changes.png). |

| Provides | Type | Description |
|---|:---|:---|
| **Status** | [Dolby.io Participant Status](types.md#dolbyio-participant-status) | The status of the participant. |
| **Participant Info** | [Dolby.io Participant Info](types.md#dolbyio-participant-info) | Information about the participant. |

---

## On Video Track Added

| Triggered by | When |
|---|:---|
| **Automatically** | A video track is added. |

| Provides | Type | Description |
|---|:---|:---|
| **Video Track** | [Dolby.io Video Track](types.md#dolbyio-video-track) | Information about the video track. |

---

## On Video Track Removed

| Triggered by | When |
|---|:---|
| **Automatically** | A video track is removed. |

| Provides | Type | Description |
|---|:---|:---|
| **Video Track** | [Dolby.io Video Track](types.md#dolbyio-video-track) | Information about the video track. |

---

## On Video Track Enabled

| Triggered by | When |
|---|:---|
| **Automatically** | A video track is enabled as a result of the video forwarding strategy. |

| Provides | Type | Description |
|---|:---|:---|
| **Video Track** | [Dolby.io Video Track](types.md#dolbyio-video-track) | Information about the video track. |

---

## On Video Track Disabled

| Triggered by | When |
|---|:---|
| **Automatically** | A video track is disabled as a result of the video forwarding strategy. |

| Provides | Type | Description |
|---|:---|:---|
| **Video Track** | [Dolby.io Video Track](types.md#dolbyio-video-track) | Information about the video track. |

---

## On Video Enabled

| Triggered by | When |
|---|:---|
| [**Dolby.io Enable Video**](functions.md#dolbyio-enable-video) | Local video is enabled. |

| Provides | Type | Description |
|---|:---|:---|
| **Video Track ID** | string | The ID of the local video track. |

---

## On Video Disabled

| Triggered by | When |
|---|:---|
| [**Dolby.io Disable Video**](functions.md#dolbyio-disable-video) | Local video is disabled. |

| Provides | Type | Description |
|---|:---|:---|
| **Video Track ID** | string | The ID of the local video track. |

---

## On Screenshare Started

| Triggered by | When |
|---|:---|
| [**Dolby.io Start Screenshare**](functions.md#dolbyio-start-screenshare) | Screenshare is started. |

| Provides | Type | Description |
|---|:---|:---|
| **Video Track ID** | string | The ID of the screenshare video track. |

---

## On Screenshare Stopped

| Triggered by | When |
|---|:---|
| [**Dolby.io Stop Screenshare**](functions.md#dolbyio-stop-screenshare) | Screenshare is stopped. |

| Provides | Type | Description |
|---|:---|:---|
| **Video Track ID** | string | The ID of the screenshare video track. |

---

## On Active Speakers Changed

| Triggered by | When |
|---|:---|
| **Automatically** | Participants start or stop speaking. |

| Provides | Type | Description |
|---|:---|:---|
| **Active Speakers** | array of strings | The IDs of the participants who are currently speaking. |

---

## On Audio Levels Changed

| Triggered by | When |
|---|:---|
| **Automatically** | Roughly every 500ms. |

| Provides | Type | Description |
|---|:---|:---|
| **Active Speakers** | array of strings | The IDs of the participants who are currently speaking. |
| **Audio Levels** | array of floats | Floating point numbers representing each participant's audio level. The order of Audio Levels corresponds to the order of Active Speakers. A value of 0.0 represents silence and a value of 1.0 represents the maximum volume. |

---

## On Screenshare Sources Received

| Triggered by | When |
|---|:---|
| [**Dolby.io Get Screenshare Sources**](functions.md#dolbyio-get-screenshare-sources) | Screen share sources are received. |

| Provides | Type | Description |
|---|:---|:---|
| **Sources** | array of [Dolby.io Screenshare Source](types.md#dolbyio-screenshare-source) | The available screen share sources. |

---

## On Audio Input Devices Received

| Triggered by | When |
|---|:---|
| [**Dolby.io Get Audio Input Devices**](functions.md#dolbyio-get-audio-input-devices) | Devices are received. |

| Provides | Type | Description |
|---|:---|:---|
| **Devices** | array of [Dolby.io Audio Device](types.md#dolbyio-audio-device) | The available devices. |

---

## On Audio Output Devices Received

| Triggered by | When |
|---|:---|
| [**Dolby.io Get Audio Output Devices**](functions.md#dolbyio-get-audio-output-devices) | Devices are received. |

| Provides | Type | Description |
|---|:---|:---|
| **Devices** | array of [Dolby.io Audio Device](types.md#dolbyio-audio-device) | The available devices. |

---

## On Current Audio Input Device Received

| Triggered by | When |
|---|:---|
| [**Dolby.io Get Current Audio Input Device**](functions.md#dolbyio-get-current-audio-input-device) | The device is received. |

| Provides | Type | Description |
|---|:---|:---|
| **Is None** | bool | If this value is "true", then there is no device set and the information in the Optional Device parameter is meaningless. |
| **Optional Device** | [Dolby.io Audio Device](types.md#dolbyio-audio-device) | The current device. |

---

## On Current Audio Output Device Received

| Triggered by | When |
|---|:---|
| [**Dolby.io Get Current Audio Output Device**](functions.md#dolbyio-get-current-audio-output-device) | The device is received. |

| Provides | Type | Description |
|---|:---|:---|
| **Is None** | bool | If this value is "true", then there is no device set and the information in the Optional Device parameter is meaningless. |
| **Optional Device** | [Dolby.io Audio Device](types.md#dolbyio-audio-device) | The current device. |

---

## On Video Devices Received

| Triggered by | When |
|---|:---|
| [**Dolby.io Get Video Devices**](functions.md#dolbyio-get-video-devices) | Devices are received. |

| Provides | Type | Description |
|---|:---|:---|
| **Devices** | array of [Dolby.io Video Device](types.md#dolbyio-video-device) | The available devices. |

---

## On Current Audio Input Device Changed

| Triggered by | When |
|---|:---|
| [**Dolby.io Set Input Device**](functions.md#dolbyio-set-input-device) | Device is changed. |
| **Automatically** | Device is changed. |

| Provides | Type | Description |
|---|:---|:---|
| **Is None** | bool | If this value is "true", then there is no device set and the information in the Optional Device parameter is meaningless. |
| **Optional Device** | [Dolby.io Audio Device](types.md#dolbyio-audio-device) | The current device. |

---

## On Current Audio Output Device Changed

| Triggered by | When |
|---|:---|
| [**Dolby.io Set Output Device**](functions.md#dolbyio-set-output-device) | Device is changed. |
| **Automatically** | Device is changed. |

| Provides | Type | Description |
|---|:---|:---|
| **Is None** | bool | If this value is "true", then there is no device set and the information in the Optional Device parameter is meaningless. |
| **Optional Device** | [Dolby.io Audio Device](types.md#dolbyio-audio-device) | The current device. |

---