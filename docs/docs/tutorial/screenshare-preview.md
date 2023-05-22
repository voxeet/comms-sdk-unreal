---
sidebar_position: 6
sidebar_label: Screen Share Preview
title: Screen Share Preview
---

This tutorial explains how to render a preview of your screen share feed.

## Prerequisites

Before you start, follow the [common setup](common-setup) tutorial.

## Set up a screen share preview

1. Find `BP_DolbyIOScreensharePreviewPlane` in the plugin's content in `Content Browser` and drag it onto the scene.

> **_NOTE:_** If you do not see this item, go to the `Content Browser` settings and tick `Show Plugin Content`.

2. Rescale the plane to the desired aspect ratio, for example, [6.4, 3.6, 1.0], and rotate it by [90, 0, 90] to make it face the player start, as in the following example:

![](../../static/img/video-plane-result.png)

If you launch the game now, assuming there is no conflicting screen share going on, you should see your screen share feed on the plane when you press "Z" and the plane should go blank when you press "X". This is because `BP_DolbyIOStarter` handles these key events as follows:

![](../../static/img/screenshare-preview-toggle.png)

This particular example will always share the first available screen share source, which is likely going to be the primary monitor screen. Selecting a screen share source is out of the scope of this tutorial as it requires some UI to parse the available screen share sources, received using [`Dolby.io Get Screenshare Sources`](../blueprints/Functions/get-screenshare-sources), into some sort of menu. The general flow however remains the same and requires getting screen share sources, choosing the desired source from the list, and starting screen share using this source.

![](../../static/img/screenshare-preview-result.png)

## How it works

`BP_DolbyIOScreensharePreviewPlane` is a sample Blueprint with a simple `Event Graph`, which handles [`On Screenshare Started`](../blueprints/Events/on-screenshare-started) and [`On Screenshare Stopped`](../blueprints/Events/on-screenshare-stopped).

![](../../static/img/screenshare-preview-eg.png)

It also contains a generic `Construction Script` to set up the plane's material:

![](../../static/img/video-plane-cs.png)

As a useful practical exercise, you can also make the plane invisible by default, visible when screen share is started, and invisible again when it is stopped.
