---
sidebar_position: 3
sidebar_label: Getting started - first conference
title: Getting started - first conference
---

The following steps present how to create a sample game which uses the plugin and connects to a demo conference. It is based on Unreal Engine 5's First Person game template.

## Step 1 - Open Event Graph
Open Unreal Editor and locate the `All/Content/Blueprints/FirstPerson` folder in the `Content Browser` window. Select the folder and open the `BP_FirstPersonCharacter` Blueprint.

## Step 2 - Add `DolbyIOObserver` component
Click the `+Add` button located in the `Components` window and select `Dolby.io Observer`.

![](../../static/img/observer-component.png)

## Step 3 - Configure access credentials
Select the added component and check its details in the `Details` panel. Scroll down to the `Events` section to see the available events. Select the [`On Token Needed`](../blueprints/Events/on-token-needed.md) event from the list by clicking the plus button next to it. After this step, an `On Token Needed (DolbyIOObserver)` node should appear in the Event Graph.

### Option A - Use client access token
1. Place a [`Dolby.io Set Token`](../blueprints/Functions/set-token.md) node and connect it to the `On Token Needed (DolbyIOObserver)` node.

![](../../static/img/settoken.png)

2. Paste your client access token copied from the Dolby.io dashboard to the [`Dolby.io Set Token`](../blueprints/Functions/set-token.md) node.

### Option B - Use app key and secret
1. Place a [`Get Dolby.io Token`](../blueprints/Functions/get-token.md) node and connect it to the `On Token Needed (DolbyIOObserver)` node.

2. Place a [`Dolby.io Set Token`](../blueprints/Functions/set-token.md) node and connect it to the [`Get Dolby.io Token`](../blueprints/Functions/get-token.md) node as shown below:

![](../../static/img/appsecret.png)

3. Provide your app key and app secret to the [`Get Dolby.io Token`](../blueprints/Functions/get-token.md) function.

## Step 4 - Connect to demo conference  

Place a [`Dolby.io Demo Conference`](../blueprints/Functions/demo-conference.md) node and connect it to the [`Dolby.io Set Token`](../blueprints/Functions/set-token.md) node as shown below:

![](../../static/img/all-token.png)

## Step 5 - Run the game
You should be able join a demo conference and hear audio from its participants.

> **_NOTE:_** If you do not, check the `Output Log` for clues - in particular, check whether the credentials are set up appropriately.