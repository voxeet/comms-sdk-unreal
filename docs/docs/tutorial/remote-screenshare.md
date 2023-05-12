---
sidebar_position: 9
sidebar_label: Rendering remote screenshare
title: Rendering remote screenshare
---

## Setup

This tutorial showcases how to render screenshare feeds coming from other participants.

Please follow the [common setup](common-setup) steps first.

Then please follow the [video plane setup](video-plane-setup) steps to get a plane to render the preview on.

## Remote screenshare

Open the video plane's `Event Graph` and add handlers for the [`On Video Track Added`](../blueprints/Events/on-video-track-added.md) and [`On Video Track Removed`](../blueprints/Events/on-video-track-removed.md) events of the `DolbyIOObserver` and wire them up as follows:

![](../../static/img/screenshare-plane-eg.png)

If you launch the game now, assuming you successfully connect to a conference with screenshare going on, you should see the screenshare feed on the plane.
