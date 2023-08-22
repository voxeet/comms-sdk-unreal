---
sidebar_position: 11
sidebar_label: Launching multiple games
title: Launching multiple games
---

If multiple games call [Set Token](../blueprints/functions.md#dolbyio-set-token) at the same time, which is likely to happen when using "Number of Players" with a value of more than 1 in the Editor, then it is likely that one of these calls will trigger an error. It is recommended to handle the error by retrying this function after a short delay - usually just one tick is enough, but if you can afford the time, it is recommended to set a longer timed delay:

![](../../static/img/multiple-games.png)

There is a known issue with the initialization and deinitialization of the plugin with the workaround above when playing games in the "Selected Viewport". The user can expect sporadic crashes to occur when launching or closing games. These issues do not occur when playing games as "Standalone Games". It is therefore recommended to use the latter option when setting "Number of Players" to more than 1.
