---
sidebar_position: 7
sidebar_label: Rendering remote screenshare
title: Rendering remote screenshare
---

This tutorial explains how to render screen share feeds coming from remote participants.

## Prerequisites

Before you start, follow the [common setup](common-setup) tutorial and the [video plane setup](video-plane-setup) steps to get a plane to render the preview on.

## Set up a remote screen share preview

1. Find `BP_DolbyIOScreensharePlane` in the plugin's content in `Content Browser` and drag it onto the scene.

> **_NOTE:_** If you do not see this item, go to the `Content Browser` settings and tick `Show Plugin Content`.

2. Rescale the plane to the desired aspect ratio, for example, [6.4, 3.6, 1.0], and rotate it by [90, 0, 90] to make it face the player start, as in the following example:

![](../../static/img/video-plane-result.png)

If you launch the game now, assuming you successfully connect to a conference with screen share enabled, you should see the screen share feed on the plane.

## How it works

`BP_DolbyIOScreensharePlane` is a sample Blueprint with a simple `Event Graph`, which handles [`On Video Track Added`](../blueprints/Events/on-video-track-added) and [`On Video Track Removed`](../blueprints/Events/on-video-track-removed).

![](../../static/img/remote-screenshare-eg.png)

It also contains a generic `Construction Script` to set up the plane's material:

![](../../static/img/video-plane-cs.png)

As a useful practical exercise, you can also make the plane invisible by default, then make it visible when screenshare is started and invisible again when it is stopped.
