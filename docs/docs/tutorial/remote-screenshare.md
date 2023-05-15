---
sidebar_position: 7
sidebar_label: Rendering remote screenshare
title: Rendering remote screenshare
---

## Setup

This tutorial showcases how to render screenshare feeds coming from other participants.

Please follow the [common setup](common-setup) steps first.

## Remote screenshare

Find `BP_DolbyIOScreensharePlane` in the plugin's content in `Content Browser` and drag it onto the scene. Preferably put it in front of the player start position, rescale it to the desired aspect ratio (here: [6.4, 3.6, 1.0]) and rotate it by [90, 0, 90] to make it face the player start. You should end up with something like this (except `BP_DolbyIOScreensharePlane` instead of `BP_DolbyIOVideoPreviewPlane`):

![](../../static/img/video-plane-result.png)

> **_NOTE:_** If you do not see this item, go to the `Content Browser` settings and tick `Show Plugin Content`.

If you launch the game now, assuming you successfully connect to a conference with screenshare going on, you should see the screenshare feed on the plane.

## How it works

`BP_DolbyIOScreensharePlane` is a sample Blueprint with a simple `Event Graph`, which handles [`On Video Track Added`](../blueprints/Events/on-video-track-added) and [`On Video Track Removed`](../blueprints/Events/on-video-track-removed).

![](../../static/img/remote-screenshare-eg.png)

It also contains a generic `Construction Script` to set up the plane's material:

![](../../static/img/video-plane-cs.png)

> **_NOTE:_** As a useful practical exercise, you can also make the plane invisible by default, then make it visible when screenshare is started and invisible again when it is stopped.
