---
sidebar_position: 6
sidebar_label: Camera preview
title: Camera preview
---

## Setup

This tutorial showcases how to render a preview of your camera feed.

Please follow the [common setup](common-setup) steps first.

Then please follow the [video plane setup](video-plane-setup) steps to get a plane to render the preview on.

## Camera preview

Open the video plane's `Event Graph` and add handlers for the [`On Video Enabled`](../blueprints/Events/on-video-enabled.md) and [`On Video Disabled`](../blueprints/Events/on-video-disabled.md) events of the `DolbyIOObserver` and wire them up as follows:

![](../../static/img/video-plane-preview.png)

This will automatically make the plane render your camera feed when it is enabled and go blank when disabled.

Let's test this by going to the `BP_FirstPersonCharacter` Blueprint and adding the following nodes to enable video with the "V" key and disable it with the "B" key:

![](../../static/img/toggle-video.png)

If you launch the game now, assuming the plugin is initialized correctly, you should see your camera feed on the plane when you press "V" and the plane should go blank when you press "B".

![](../../static/img/camera-preview.png)

> **_NOTE:_** As a useful practical exercise, you can also make the plane invisible initially, then make it visible when video is enabled and invisible again when it is disabled.
