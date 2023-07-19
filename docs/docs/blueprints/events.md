# Events

## On Active Speakers Changed

Triggered automatically when participants start or stop speaking.

#### Data provided
| Provides | Type | Description |
|---|:---|:---|
| **Active Speakers** | array of strings | The IDs of the participants who are currently speaking. |

---

## On Audio Input Devices Received

Triggered by [**Dolby.io Get Audio Input Devices**](functions.md#dolbyio-get-audio-input-devices) when devices are received.

#### Data provided
| Provides | Type | Description |
|---|:---|:---|
| **Devices** | array of [Dolby.io Audio Device](types.mdx#dolbyio-audio-device) | The available devices. |

---

## On Audio Levels Changed

Triggered automatically roughly every 500ms.

#### Data provided
| Provides | Type | Description |
|---|:---|:---|
| **Active Speakers** | array of strings | The IDs of the participants who are currently speaking. |
| **Audio Levels** | array of floats | Floating point numbers representing each participant's audio level. The order of Audio Levels corresponds to the order of Active Speakers. A value of 0.0 represents silence and a value of 1.0 represents the maximum volume. |

---

## On Audio Output Devices Received

Triggered by [**Dolby.io Get Audio Output Devices**](functions.md#dolbyio-get-audio-output-devices) when devices are received.

#### Data provided
| Provides | Type | Description |
|---|:---|:---|
| **Devices** | array of [Dolby.io Audio Device](types.mdx#dolbyio-audio-device) | The available devices. |

---

## On Connected

Triggered by [**Dolby.io Connect**](functions.md#dolbyio-connect) or [**Dolby.io Demo Conference**](functions.md#dolbyio-demo-conference) when the client is successfully connected to the conference.

#### Data provided
| Provides | Type | Description |
|---|:---|:---|
| **Local Participant ID** | string | The ID of the local participant. |
| **Conference ID** | string | The ID of the conference. |

---

## On Current Audio Input Device Changed

Triggered by [**Dolby.io Set Input Device**](functions.md#dolbyio-set-input-device) or automatically when the device is changed.

#### Data provided
| Provides | Type | Description |
|---|:---|:---|
| **Is None** | bool | If this value is "true", then there is no device set and the information in the Optional Device parameter is meaningless. |
| **Optional Device** | [Dolby.io Audio Device](types.mdx#dolbyio-audio-device) | The current device. |

---

## On Current Audio Input Device Received

Triggered by [**Dolby.io Get Current Audio Input Device**](functions.md#dolbyio-get-current-audio-input-device) when the device is received.

#### Data provided
| Provides | Type | Description |
|---|:---|:---|
| **Is None** | bool | If this value is "true", then there is no device set and the information in the Optional Device parameter is meaningless. |
| **Optional Device** | [Dolby.io Audio Device](types.mdx#dolbyio-audio-device) | The current device. |

---

## On Current Audio Output Device Changed

Triggered by [**Dolby.io Set Output Device**](functions.md#dolbyio-set-output-device) or automatically when the device is changed.

#### Data provided
| Provides | Type | Description |
|---|:---|:---|
| **Is None** | bool | If this value is "true", then there is no device set and the information in the Optional Device parameter is meaningless. |
| **Optional Device** | [Dolby.io Audio Device](types.mdx#dolbyio-audio-device) | The current device. |

---

## On Current Audio Output Device Received

Triggered by [**Dolby.io Get Current Audio Output Device**](functions.md#dolbyio-get-current-audio-output-device) when the device is received.

#### Data provided
| Provides | Type | Description |
|---|:---|:---|
| **Is None** | bool | If this value is "true", then there is no device set and the information in the Optional Device parameter is meaningless. |
| **Optional Device** | [Dolby.io Audio Device](types.mdx#dolbyio-audio-device) | The current device. |

---

## On Current Video Device Received

Triggered by [**Dolby.io Get Current Video Device**](functions.md#dolbyio-get-current-video-device) when the device is received.

#### Data provided
| Provides | Type | Description |
|---|:---|:---|
| **Is None** | bool | If this value is "true", then there is no device set and the information in the Optional Device parameter is meaningless. |
| **Optional Device** | [Dolby.io Video Device](types.mdx#dolbyio-video-device) | The current device. |

---

## On Disconnected

Triggered by [**Dolby.io Disconnect**](functions.md#dolbyio-disconnect) when the client is disconnected from the conference. The event is also triggered automatically after receiving an error that disconnected the client from the conference.

---

## On Error

Triggered by functions if errors occur.

#### Data provided
| Provides | Type | Description |
|---|:---|:---|
| **Error Msg** | string | The error message. |

---

## On Initialized

Triggered by [**Dolby.io Set Token**](functions.md#dolbyio-set-token) when the plugin is successfully initialized. After receiving this event, the plugin is ready for use. You can now, for example, call the [Dolby.io Connect](functions.md#dolbyio-connect) function.

---

## On Participant Added

Triggered automatically when a remote participant is added to the conference.

#### Data provided
| Provides | Type | Description |
|---|:---|:---|
| **Status** | [Dolby.io Participant Status](types.mdx#dolbyio-participant-status) | The status of the participant. |
| **Participant Info** | [Dolby.io Participant Info](types.mdx#dolbyio-participant-info) | Information about the participant. |

---

## On Participant Updated

Triggered automatically when a remote participant's status is updated. For more information, refer to the [graph of possible status changes](../../static/img/participant-status-changes.png).

#### Data provided
| Provides | Type | Description |
|---|:---|:---|
| **Status** | [Dolby.io Participant Status](types.mdx#dolbyio-participant-status) | The status of the participant. |
| **Participant Info** | [Dolby.io Participant Info](types.mdx#dolbyio-participant-info) | Information about the participant. |

---

## On Screenshare Sources Received

Triggered by [**Dolby.io Get Screenshare Sources**](functions.md#dolbyio-get-screenshare-sources) when screen share sources are received.

#### Data provided
| Provides | Type | Description |
|---|:---|:---|
| **Sources** | array of [Dolby.io Screenshare Source](types.mdx#dolbyio-screenshare-source) | The available screen share sources. |

---

## On Screenshare Started

Triggered by [**Dolby.io Start Screenshare**](functions.md#dolbyio-start-screenshare) when screenshare is started.

#### Data provided
| Provides | Type | Description |
|---|:---|:---|
| **Video Track ID** | string | The ID of the screenshare video track. |

---

## On Screenshare Stopped

Triggered by [**Dolby.io Stop Screenshare**](functions.md#dolbyio-stop-screenshare) when screenshare is stopped.

#### Data provided
| Provides | Type | Description |
|---|:---|:---|
| **Video Track ID** | string | The ID of the screenshare video track. |

---

## On Token Needed

Triggered automatically when an initial or refreshed [client access token](https://docs.dolby.io/communications-apis/docs/overview-developer-tools#client-access-token) is needed, which happens when the Dolby.io Subsystem is initialized or when a refresh token is requested. After receiving this event, obtain a token for your Dolby.io application and call the Dolby.io Set Token function.

---

## On Token Obtained

Triggered by [**Dolby.io Get Token From URL**](functions.md#dolbyio-get-token-from-url) and [**Get Dolby.io Token**](functions.md#get-dolbyio-token) when a client access token is obtained.

#### Data provided
| Provides | Type | Description |
|---|:---|:---|
| **Token** | string | The client access token. |

---

## On Video Devices Received

Triggered by [**Dolby.io Get Video Devices**](functions.md#dolbyio-get-video-devices) when devices are received.

#### Data provided
| Provides | Type | Description |
|---|:---|:---|
| **Devices** | array of [Dolby.io Video Device](types.mdx#dolbyio-video-device) | The available devices. |

---

## On Video Disabled

Triggered by [**Dolby.io Disable Video**](functions.md#dolbyio-disable-video) when local video is disabled.

#### Data provided
| Provides | Type | Description |
|---|:---|:---|
| **Video Track ID** | string | The ID of the local video track. |

---

## On Video Enabled

Triggered by [**Dolby.io Enable Video**](functions.md#dolbyio-enable-video) when local video is enabled.

#### Data provided
| Provides | Type | Description |
|---|:---|:---|
| **Video Track ID** | string | The ID of the local video track. |

---

## On Video Track Added

Triggered automatically when a video track is added.

#### Data provided
| Provides | Type | Description |
|---|:---|:---|
| **Video Track** | [Dolby.io Video Track](types.mdx#dolbyio-video-track) | Information about the video track. |

---

## On Video Track Disabled

Triggered automatically when a video track is disabled as a result of the video forwarding strategy.

#### Data provided
| Provides | Type | Description |
|---|:---|:---|
| **Video Track** | [Dolby.io Video Track](types.mdx#dolbyio-video-track) | Information about the video track. |

---

## On Video Track Enabled

Triggered automatically when a video track is enabled as a result of the video forwarding strategy.

#### Data provided
| Provides | Type | Description |
|---|:---|:---|
| **Video Track** | [Dolby.io Video Track](types.mdx#dolbyio-video-track) | Information about the video track. |

---

## On Video Track Removed

Triggered automatically when a video track is removed.

#### Data provided
| Provides | Type | Description |
|---|:---|:---|
| **Video Track** | [Dolby.io Video Track](types.mdx#dolbyio-video-track) | Information about the video track. |

---
