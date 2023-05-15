---
sidebar_position: 4
sidebar_label: Camera preview
title: Camera preview
---

## Setup

This tutorial showcases how to render a preview of your camera feed.

Please follow the [common setup](common-setup) steps first.

## Camera preview

Find `BP_DolbyIOVideoPreviewPlane` in the plugin's content in `Content Browser` and drag it onto the scene. Preferably put it in front of the player start position, rescale it to the desired aspect ratio (here: [6.4, 3.6, 1.0]) and rotate it by [90, 0, 90] to make it face the player start. You should end up with something like this:

![](../../static/img/video-plane-result.png)

> **_NOTE:_** If you do not see this item, go to the `Content Browser` settings and tick `Show Plugin Content`.

Let's test this by going to the `BP_FirstPersonCharacter` Blueprint and adding the following nodes to enable video with the "V" key and disable it with the "B" key:

![](../../static/img/camera-preview-toggle.png)

If you launch the game now, assuming the plugin is initialized correctly, you should see your camera feed on the plane when you press "V" and the plane should go blank when you press "B".

![](../../static/img/camera-preview-result.png)

## How it works

`BP_DolbyIOVideoPreviewPlane` is a sample Blueprint with a simple `Event Graph`, which handles [`On Video Enabled`](../blueprints/Events/on-video-enabled) and [`On Video Disabled`](../blueprints/Events/on-video-enabled).

![](../../static/img/camera-preview-eg.png)

It also contains a generic `Construction Script` to set up the plane's material:

![](../../static/img/video-plane-cs.png)

> **_NOTE:_** As a useful practical exercise, you can also make the plane invisible by default, then make it visible when video is enabled and invisible again when it is disabled.
