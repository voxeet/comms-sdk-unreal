---
sidebar_label: On Video Track Added
title: On Video Track Added
---
Triggered when a video track is added. Provides information about the video track in its argument. This information consists of the track ID, the ID of the participant from whom the track is coming and a boolean indicating whether this is a screenshare video track or a camera video track.

> **_NOTE:_** NOT triggered when the user [enables their own camera](../Functions/enable-video) or [starts their own screenshare](../Functions/start-screenshare). For events related to these actions, see [On Video Enabled](on-video-enabled) and [On Screenshare Started](on-screenshare-started) respectively.

For example usage, see the [remote video rendering tutorial](../../tutorial/remote-video).

Another example:

![](../../../static/img/example-on-video-track-added.png)