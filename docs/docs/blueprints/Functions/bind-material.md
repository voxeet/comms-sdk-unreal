---
sidebar_label: Bind Material
title: Bind Material
---
Binds a dynamic material instance to hold the frames of the given video track. The plugin will update the material's texture parameter named `DolbyIO Frame` with the necessary data, therefore the material should have such a parameter to be usable. Automatically unbinds the material from all other tracks, but it is possible to bind multiple materials to the same track. Has no effect if the track does not exist at the moment the function is called, therefore it should usually be called as a response to the [On Video Track Added](../Events/on-video-track-added) event.

For example usage, see the [camera preview tutorial](../../tutorial/camera-preview).