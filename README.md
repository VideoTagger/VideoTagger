# VideoTagger

![VideoTagger](/resources/images/preview.png?raw=true "VideoTagger")

## Getting started

Download the repository with:
```shell
git clone --recursive https://github.com/VideoTagger/VideoTagger
```

If the repository was cloned non-recursively run:
```shell
git submodule update --init
```

## Build Requirements
- Python 3.12+ (tested with 3.12.6 and 3.13.0)
- Visual Studio 2022 (on Windows)

> [!Important]
> Both Linux and macOS require SDL2 version 2.0.17 or later to build properly.
Windows comes with prebuilt binaries.

## Windows - Visual Studio
Generate project files with:
```shell
scripts/win-gen-projects.cmd
```

Build the projects by opening the `Visual Studio` solution file and building with desired configuration.

## Linux - Makefile
Install Required packages
```
build-essential pkg-config libsdl2-dev libavcodec-dev libavformat-dev libswscale-dev python3-dev libgtk-3-dev libglib2.0-dev libgtk2.0-dev
```

Generate project files with:
```shell
./scripts/linux-gen-projects.sh
```

Build the projects by running:
```shell
make config=<BUILD_CONFIG>
```
Replace `<BUILD_CONFIG>` with one of:
- `debug_x86_64`
- `release_x86_64`
- `shipping_x86_64`

## Xcode (macOS)
> [!Warning]
> Building on macOS is untested

Generate project files with:
```shell
scripts/macos-gen-projects.sh
```

Build the projects by opening the `Xcode` project file and building with desired configuration.

## Third party libraries
- [SDL2](https://github.com/libsdl-org/SDL)
- [ImGui](https://github.com/ocornut/imgui)
- [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo)
- [Native File Dialog Extended](https://github.com/btzy/nativefiledialog-extended/tree/master)
- [fmt](https://github.com/fmtlib/fmt)
- [Nlohmann Json](https://github.com/nlohmann/json)
- [utf8.h](https://github.com/sheredom/utf8.h)
- [pybind11](https://github.com/pybind/pybind11)
- [FFmpeg](https://ffmpeg.org/)
