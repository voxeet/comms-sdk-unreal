---
sidebar_position: 5
sidebar_label: Camera preview
title: Camera preview
---

This tutorial explains how to render a preview of your camera feed.

## Prerequisites

Before you start, [set up](common-setup) your project and follow the [video plane setup](video-plane-setup) to get a plane to render the preview on.

## Set up camera preview

1. Open the video plane's `Event Graph` and add handlers for the [`On Video Enabled`](../blueprints/Events/on-video-enabled) and [`On Video Disabled`](../blueprints/Events/on-video-disabled) events of the `DolbyIOObserver` and wire them up as in the following example:

![](../../static/img/camera-preview-eg.png)

This automatically makes the plane render your camera feed when it is enabled and go blank when disabled.

2. Test the camera preview by going to the `BP_FirstPersonCharacter` Blueprint and adding the following nodes to enable video with the "V" key and disable it with the "B" key:

![](../../static/img/camera-preview-toggle.png)

If you launch the game now, assuming the plugin is initialized correctly, you should see your camera feed on the plane when you press "V" and the plane should go blank when you press "B".

![](../../static/img/camera-preview-result.png)

As a useful practical exercise, you can also make the plane invisible by default, visible when video is enabled, and invisible again when it is disabled.
