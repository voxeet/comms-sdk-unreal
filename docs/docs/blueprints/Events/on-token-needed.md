---
sidebar_label: On Token Needed
title: On Token Needed
---
Triggered when an initial or refreshed client access token is needed, which happens when the Dolby.io subsystem is initialized or when a refresh token is requested.

After receiving this event, obtain a token for your Dolby.io application and call the [Set Token](../Functions/set-token) function.

For example usage, see the [first conference tutorial](../../tutorial/first-conference).