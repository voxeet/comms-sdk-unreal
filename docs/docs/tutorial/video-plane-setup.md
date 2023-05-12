---
sidebar_position: 4
sidebar_label: Video plane setup
title: Video plane setup
---

For all the video-related tutorials, we need some surfaces to display the videos on. This can be accomplished in multiple ways (e.g. in a HUD, in widgets), but for simplicity we will use plain old planes to render the video feeds. Drag and drop a plane from the the `Place Actors` window onto the scene. Preferably put it in front of the player start position, rescale it to the desired aspect ratio (here: [6.4, 3.6, 1.0]) and rotate it by [90, 0, 90] to make it face the player start. You should end up with something like this:

![](../../static/img/video-plane-result.png)

The plane also needs an appropriate material for the videos to be displayed correctly. Follow [these steps](common-setup#video-material) to set up the material. Then, click on the highlighted button to convert the plane into a Blueprint:

![](../../static/img/video-plane-blueprintize.png)

Edit the newly-created Blueprint and place the following nodes in the `Construction Script`:

![](../../static/img/video-plane-cs.png)

That is, create a dynamic material instance of type `M_DolbyIOVideo`, promote it to a variable and set it as the plane's material.

Turning to the `Event Graph`, you may delete anything put there automatically and instead add `Dolby.io Observer` as a component:

![](../../static/img/first-conf-observer-component.png)
