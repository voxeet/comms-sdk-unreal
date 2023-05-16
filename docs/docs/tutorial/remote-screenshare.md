---
sidebar_position: 8
sidebar_label: Rendering remote screenshare
title: Rendering remote screenshare
---

This tutorial explains how to render screen share feeds coming from remote participants.

## Prerequisites

Before you start, follow the [common setup](common-setup) tutorial and the [video plane setup](video-plane-setup) steps to get a plane to render the preview on.

## Set up a remote screen share preview

Open the video plane's `Event Graph` and add handlers for the [`On Video Track Added`](../blueprints/Events/on-video-track-added) and [`On Video Track Removed`](../blueprints/Events/on-video-track-removed) events of the `DolbyIOObserver` and wire them up as in the following example:

![](../../static/img/remote-screenshare-eg.png)

If you launch the game now, assuming you successfully connect to a conference with screen share enabled, you should see the screen share feed on the plane.
