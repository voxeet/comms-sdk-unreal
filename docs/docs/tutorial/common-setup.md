---
sidebar_position: 3
sidebar_label: Common Tutorial Setup
title: Common Tutorial Setup
---

This tutorial explains how to initialize the plugin.

## Prerequisites

The tutorials and samples are designed to work with the Unreal Engine 5 First Person game template.

The sample Blueprints included with the plugin are designed in Unreal Engine 5 and will not work with Unreal Engine 4. You can still use Unreal Engine 4 to follow the tutorials but you will not be able to use the sample Blueprints and will need to create them from scratch.

## Initialize the plugin

You can initialize the plugin from multiple places, such as the game instance or the level Blueprint, but in this example, we will use the First Person Character Blueprint since it is already in the template. 

1. In the Content Browser, open `All -> Content -> FirstPerson -> Blueprints -> BP_FirstPersonCharacter`. If you set up the project from a C++ template, click `Open Full Blueprint Editor`.

2. Add `Dolby.io Observer` as a component and add a handler for the [`On Token Needed`](../blueprints/Events/on-token-needed) event. Wire the nodes as presented in the following example and paste your client access token into the [`Dolby.io Set Token`](../blueprints/Functions/set-token) node.

![](../../static/img/common-setup-connect.png)

In development, you may also use the [`Get Dolby.io Token`](../blueprints/Functions/get-token) helper to use your app key and secret instead.

After this step, you should connect to a conference that has a name that you specified in the [`Dolby.io Connect`](../blueprints/Functions/connect) node upon starting your game. If you cannot connect, follow the [Getting started - first conference](first-conference) tutorial first.
