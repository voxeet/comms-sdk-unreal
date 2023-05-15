---
sidebar_position: 3
sidebar_label: Common tutorial setup
title: Common tutorial setup
---

## Prerequisites

All of the tutorials will be based on a scene set up using the Unreal Engine 5 First Person game template.

## Initialization

For all tutorials, we need to initialize the plugin. We could do this from multiple places (such as the game instance, the level Blueprint, etc.), but for this example we will use the First Person Character Blueprint since it is already in the template and will do just fine. Therefore, in the Content Browser, open `All -> Content -> FirstPerson -> Blueprints -> BP_FirstPersonCharacter`.

> **_NOTE:_** If you set up the project from a C++ template, click `Open Full Blueprint Editor`.

Add `Dolby.io Observer` as a component and add a handler for the [`On Token Needed`](../blueprints/Events/on-token-needed) event. Wire the nodes as presented below, pasting your client access token in the [`Dolby.io Set Token`](../blueprints/Functions/set-token) node:

![](../../static/img/common-setup-connect.png)

> **_NOTE:_** In development, you may also use the [`Get Dolby.io Token`](../blueprints/Functions/get-token) helper to use your app key and secret instead.

If set up correctly, this should connect you to the conference name specified in the [`Dolby.io Connect`](../blueprints/Functions/connect) node upon game start.

> **_NOTE:_** If this does not work, try following the [Getting started - first conference](first-conference) tutorial first.
