---
sidebar_position: 6
sidebar_label: Screenshare preview
title: Screenshare preview
---

This tutorial explains how to render a preview of your screen share feed.

## Prerequisites

Before you start, follow the [common setup](common-setup) tutorial and the [video plane setup](video-plane-setup) steps to get a plane to render the preview on.

## Set up a screen share preview

1. Open the video plane's `Event Graph` and add handlers for the [`On Screenshare Started`](../blueprints/Events/on-screenshare-started) and [`On Screenshare Stopped`](../blueprints/Events/on-screenshare-stopped) events of the `DolbyIOObserver` and wire them up as in the following example:

![](../../static/img/screenshare-preview-eg.png)

This will automatically make the plane render your screen share feed when it is started and go blank when stopped.

2. Test the preview by going to the `BP_FirstPersonCharacter` Blueprint and adding the following nodes to enable video with the "Z" key and disable it with the "X" key:

![](../../static/img/screenshare-preview-toggle.png)

If you launch the game now, assuming you connect to the conference successfully and there is no conflicting screen share going on, you should see your screen share feed on the plane when you press "Z" and the plane should go blank when you press "X".

![](../../static/img/screenshare-preview-result.png)

This particular example will always share the first available screen share source, which is likely going to be the primary monitor screen. Selecting a screen share source is out of the scope of this tutorial as it requires some UI to parse the available screen share sources, received using [`Dolby.io Get Screenshare Sources`](../blueprints/Functions/get-screenshare-sources), into some sort of menu. The general flow however remains the same and requires getting screen share sources, choosing the desired source from the list, and starting screen share using this source.

As a useful practical exercise, you can also make the plane invisible by default, visible when screen share is started, and invisible again when it is stopped.
