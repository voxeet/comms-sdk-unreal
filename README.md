[![Build Unreal Plugin](https://github.com/DolbyIO/comms-sdk-unreal/actions/workflows/build.yml/badge.svg)](https://github.com/DolbyIO/comms-sdk-unreal/actions/workflows/build.yml)
[![Documentation](https://github.com/DolbyIO/comms-sdk-unreal/actions/workflows/deploy-to-github-pages.yml/badge.svg)](https://github.com/DolbyIO/comms-sdk-unreal/actions/workflows/deploy-to-github-pages.yml)
[![License](https://img.shields.io/github/license/DolbyIO/comms-sdk-unreal)](LICENSE)

# Dolby.io Virtual Worlds plugin for Unreal Engine

With the Dolby.io Virtual Worlds plugin for Unreal Engine, you can easily integrate [Dolby.io](https://dolby.io) Spatial Audio, powered by Dolby Atmos technology into your virtual world applications.

You can find the plugin documentation here: [Online documentation](https://api-references.dolby.io/comms-sdk-unreal/)

## Supported platforms

The plugin is compatible with the following operating systems:
- Windows 10+
- macOS 10.14+ if you use UE4
- macOS 12+ if you use UE5

## Prerequisites

Before you start, make sure that you have:
- Unreal Engine 4.27, 5.0 or 5.1 installed on your computer. For more details, see the [Unreal Engine download](https://www.unrealengine.com/en-US/download) page and the [4.27](https://docs.unrealengine.com/4.27/en-US/Basics/InstallingUnrealEngine/) or [5.0](https://docs.unrealengine.com/5.0/en-US/installing-unreal-engine/)/[5.1](https://docs.unrealengine.com/5.1/en-US/installing-unreal-engine/) installation manuals.
- A [Dolby.io](https://dolby.io) account. If you do not have an account, [sign up](https://dolby.io/signup) for free.
- A client access token or an app key and an app secret copied from the Communications & Media [Dolby.io Dashboard](https://dashboard.dolby.io/).

## Installation

1. Make sure that your project contains a `Plugins` folder in the root of your game folder. If you do not have this folder, create it.
2. Download the latest plugin [release](https://github.com/DolbyIO/comms-sdk-unreal/releases) for your platform and unpack the plugin to the `Plugins` folder.
3. Launch your project using the Unreal Editor and select `Edit > Plugins` from the menu to enable the plugin manually. This step requires restarting the editor.
