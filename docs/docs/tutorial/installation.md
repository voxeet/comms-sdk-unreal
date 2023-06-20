---
sidebar_position: 1
sidebar_label: Installation
title: Installation
---

Download and enable the plugin to be able to use it in the Unreal Editor. You can either use the binary or build the plugin from source.

- On macOS, you need to additionally [obtain permissions](obtain-permissions) to use your microphone and camera.
- On Ubuntu, you need to have libc++ installed: `sudo apt install libc++-dev`.

## Enable the plugin using the binary
1. Make sure that your project contains a `Plugins` folder in the root of your game folder. If you do not have this folder, create it.
2. Download the latest plugin [release](https://github.com/DolbyIO/comms-sdk-unreal/releases) for your platform and unpack the plugin to the `Plugins` folder.
3. Launch your project using the Unreal Editor and select `Edit > Plugins` from the menu to enable the plugin manually. This step requires restarting the editor.

## Build the plugin from source
1. Make sure that your project contains a `Plugins` folder in the root of your game folder. If you do not have this folder, create it.
2. Download the `DolbyIO` folder from [GitHub](https://github.com/DolbyIO/comms-sdk-unreal/tree/main/DolbyIO) and add it to the `Plugins` folder.
3. Download the latest Dolby.io Communications C++ SDK [release](https://github.com/DolbyIO/comms-sdk-cpp/releases) for your platform (use the "universal" package for macOS or the "clang10-libc++10" package for Ubuntu) and unzip it. The unpacked package should contain the `sdk-release` folder for Windows and macOS or the `sdk-release-ubuntu-20.04-clang10-libc++10` folder for Ubuntu.
4. Move the unzipped folder to the `{YourGameRoot}/Plugins/DolbyIO` folder.
5. Regenerate project files and make sure that the Unreal Editor is closed.
6. Build the project using the `Development Editor` configuration.
