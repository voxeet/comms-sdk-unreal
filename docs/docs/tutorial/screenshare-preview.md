---
sidebar_position: 6
sidebar_label: Screenshare preview
title: Screenshare preview
---

## Setup

This tutorial showcases how to render a preview of your screenshare feed.

Please follow the [common setup](common-setup) steps first.

Then please follow the [video plane setup](video-plane-setup) steps to get a plane to render the preview on.

## Screenshare preview

Open the video plane's `Event Graph` and add handlers for the [`On Screenshare Started`](../blueprints/Events/on-screenshare-started) and [`On Screenshare Stopped`](../blueprints/Events/on-screenshare-stopped) events of the `DolbyIOObserver` and wire them up as follows:

![](../../static/img/screenshare-preview-eg.png)

This will automatically make the plane render your screenshare feed when it is started and go blank when stopped.

Let's test this by going to the `BP_FirstPersonCharacter` Blueprint and adding the following nodes to enable video with the "Z" key and disable it with the "X" key:

![](../../static/img/screenshare-preview-toggle.png)

If you launch the game now, assuming you connect to the conference successfully and there is conflicting screenshare going on, you should see your screenshare feed on the plane when you press "Z" and the plane should go blank when you press "X".

![](../../static/img/screenshare-preview-result.png)

> **_NOTE:_** This particular example will always share the first available screenshare source, which is likely going to be the primary monitor screen. Selecting an appropriate screenshare source is out of the scope of this tutorial as it will likely require some UI to parse the available screenshare sources (received using [`Dolby.io Get Screenshare Sources`](../blueprints/Functions/get-screenshare-sources)) into some sort of menu. The general flow however remains the same: get screenshare sources, choose desired source from list, start screenshare using this source.

> **_NOTE:_** As a useful practical exercise, you can also make the plane invisible by default, then make it visible when screenshare is started and invisible again when it is stopped.
