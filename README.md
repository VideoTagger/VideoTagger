# VideoTagger

![VideoTagger](https://raw.githubusercontent.com/VideoTagger/VideoTagger/refs/heads/main/resources/images/preview.png "VideoTagger")

## Getting started

To download the repository run:
```shell
git clone --recursive https://github.com/VideoTagger/VideoTagger
```

If the repository was cloned non-recursively run:
```shell
git submodule update --init
```

## Requirements
- Python (tested with versions 3.12.6 and 3.13.0)

## Building
> [!Important]
> Both Linux and macOS require SDL2 version 2.0.17 or later to build properly.
Windows comes with prebuilt binaries.

- ### Windows - Visual Studio
	> [!Note]
	> Visual Studio 2019 or 2022 is recommended

	Generate project files with:
	```shell
	scripts/win-gen-projects.cmd
	```

	Build the projects by opening the `Visual Studio` solution file and building with desired configuration.


- ### Linux - Makefile
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

- ### Xcode (macOS)
	Generate project files with:
	```shell
	scripts/macos-gen-projects.sh
	```

	Build the projects by opening the `Xcode` project file and building with desired configuration.

	> [!Warning]
	> Building on macOS is untested

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
