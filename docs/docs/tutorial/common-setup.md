---
sidebar_position: 3
sidebar_label: Common Tutorial Setup
title: Common Tutorial Setup
---

This tutorial explains how to initialize the plugin and connect to a conference, which is a common prerequisite for other tutorials.

## Prerequisites

The tutorials and samples are designed to work with the Unreal Engine 5 First Person game template.

The sample Blueprints included with the plugin are designed in Unreal Engine 5 and will not work with Unreal Engine 4. You can still use Unreal Engine 4 to follow the tutorials but you will not be able to use the sample Blueprints and will need to create them from scratch.

## Getting started

1. Find `BP_DolbyIOStarter` in the plugin's content in `Content Browser` and drag it onto the scene.

> **_NOTE:_** If you do not see this item, go to the `Content Browser` settings and tick `Show Plugin Content`.

2. Make sure the newly-placed actor is selected and enter your client access token or app key and secret combination in the `Details` panel.

You can modify other parameters of this actor as you see fit, but using the demo conference does not make sense for some tutorials, because it is an audio-only conference.

If you launch the game now, assuming the credentials are set up appropriately, you should connect to a conference.

> **_NOTE:_** `BP_DolbyIOStarter` is a sample Blueprint which handles several "key pressed" events and as such is designed to work the template game only, because it will likely override key bindings in more advanced games. It is meant to be used as an example on how to get started, not for use in production.
