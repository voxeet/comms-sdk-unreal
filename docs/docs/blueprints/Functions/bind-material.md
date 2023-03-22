---
sidebar_label: Bind Material
title: Bind Material
---
Binds a dynamic material instance to hold a participant's video frames. The plugin will update the material's texture parameter named "DolbyIO Frame" with the necessary data, therefore the material should have such a parameter to be usable. Automatically unbinds the material from all other participants, but it is possible to bind multiple materials to the same participant. Has no effect if there is no video from the participant at the moment the function is called, therefore it should usually be called as a response to the [On Video Track Added](../Events/on-video-track-added) event.

![Sample](../../../static/img/bind_material.png)