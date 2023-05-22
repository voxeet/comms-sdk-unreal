---
sidebar_position: 5
sidebar_label: Camera Preview
title: Camera Preview
---

This tutorial explains how to render a preview of your camera feed.

## Prerequisites

Before you start, [set up](common-setup) your project.

## Set up camera preview

1. Find `BP_DolbyIOVideoPreviewPlane` in the plugin's content in `Content Browser` and drag it onto the scene.

> **_NOTE:_** If you do not see this item, go to the `Content Browser` settings and tick `Show Plugin Content`.

2. Rescale the plane to the desired aspect ratio, for example, [6.4, 3.6, 1.0], and rotate it by [90, 0, 90] to make it face the player start, as in the following example:

![](../../static/img/video-plane-result.png)

If you launch the game now, you should see your camera feed on the plane when you press "V" and the plane should go blank when you press "B". This is because `BP_DolbyIOStarter` handles these key events as follows:

![](../../static/img/camera-preview-toggle.png)

## How it works

`BP_DolbyIOVideoPreviewPlane` is a sample Blueprint with a simple `Event Graph`, which handles [`On Video Enabled`](../blueprints/Events/on-video-enabled) and [`On Video Disabled`](../blueprints/Events/on-video-enabled).

![](../../static/img/camera-preview-eg.png)

It also contains a generic `Construction Script` to set up the plane's material:

![](../../static/img/video-plane-cs.png)

As a useful practical exercise, you can also make the plane invisible by default, then make it visible when video is enabled and invisible again when it is disabled.
