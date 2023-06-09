---
sidebar_label: Start Screenshare
title: Start Screenshare
---
Starts sharing a screen using a given source. Users should make use of the function's parameters to optimize for the content they are sharing. For example, for sharing dynamic content like a YouTube video, the ideal settings are MaxResolution = DownscaleTo1080p, EncoderHint = Fluid, DownscaleQuality = High. The source should be obtained using the [Get Screenshare Sources](../Functions/get-screenshare-sources) function. The function triggers the [On Screenshare Started](../Events/on-screenshare-started) event if successful.

For example usage, see the [screenshare preview tutorial](../../tutorial/screenshare-preview).
