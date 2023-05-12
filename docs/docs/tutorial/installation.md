---
sidebar_position: 1
sidebar_label: Installation
title: Installation
---

Download and enable the plugin to be able to use it in the Unreal Editor. You can either [use the binary](#enable-the-plugin-using-the-binary) or [build the plugin from source](#build-the-plugin-from-source). On macOS, you need to additionally [obtain permissions](#obtaining-permissions) to use your microphone and camera.

## Enable the plugin using the binary
1. Make sure that your project contains a `Plugins` folder in the root of your game folder. If you do not have this folder, create it.
2. Download the latest plugin [release](https://github.com/DolbyIO/comms-sdk-unreal/releases) for your platform and unpack the plugin to the `Plugins` folder.
3. Launch your project using the Unreal Editor and select `Edit > Plugins` from the menu to enable the plugin manually. This step requires restarting the editor.

## Build the plugin from source
1. Make sure that your project contains a `Plugins` folder in the root of your game folder. If you do not have this folder, create it.
2. Download the `DolbyIO` folder from [GitHub](https://github.com/DolbyIO/comms-sdk-unreal/tree/main/DolbyIO) and add it to the `Plugins` folder.
3. Download the Dolby.io Communications C++ SDK [2.4.2](https://github.com/DolbyIO/comms-sdk-cpp/releases/tag/2.4.2) release for your platform and unzip it.
The unpacked macOS package should contain the `sdk-release-x86` and the `sdk-release-arm` sub-folders, and the downloaded Windows package should be visible as the `sdk-release` folder. The `sdk-release-x86` package is designed for Macs with an Intel processor and `sdk-release-arm` is for Macs with Apple silicon.
4. Move the proper unzipped folder to the `{YourGameRoot}/Plugins/DolbyIO` folder.
5. Regenerate project files and make sure that the Unreal Editor is closed.
6. Build the project using the Development Editor configuration.
