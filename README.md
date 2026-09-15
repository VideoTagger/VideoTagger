# VideoTagger

![VideoTagger](/resources/images/preview.png "VideoTagger")

## Getting started

Download the repository with:
```shell
git clone --recursive https://github.com/VideoTagger/VideoTagger
```

If the repository was cloned non-recursively run:
```shell
git submodule update --init
```

### Build Requirements
- Python 3.12+ (tested with 3.12.6 and 3.13.0)
- CMake 3.24+

## Initial setup
Install `uv` package manager:
```shell
# On Windows
powershell -ExecutionPolicy ByPass -c "irm https://astral.sh/uv/install.ps1 | iex"

# On macOS and Linux
curl -LsSf https://astral.sh/uv/install.sh | sh
```

On Linux install the following packages:
```shell
build-essential pkg-config cmake ninja-build python3 python3-pip libsdl2-dev libavutil-dev libavcodec-dev libavformat-dev libswscale-dev python3-dev libgtk-3-dev libglib2.0-dev libgtk2.0-dev libgl1-mesa-dev libssl-dev libopencv-dev
```

In the root directory run:
```shell
uv run ./scripts/setup.py
```

> [!Warning]
> Building on macOS is untested

## Build using CMake
```shell
# On Windows it might be necessary to activate the developer command prompt first, you can do it by running:
/path/to/vcvars64.bat

cmake --preset=<BUILD_PRESET> && cmake --build --target install --preset=<BUILD_PRESET>
```

Replace `<BUILD_PRESET>` with one of the presets:
- `<SYSTEM>-x64-debug`
- `<SYSTEM>-x64-release`
- `<SYSTEM>-x64-shipping`

where `<SYSTEM>` is `windows`/`linux`/`macos`

You can also show all available presets with
```shell
cmake --list-presets
cpack --list-presets
```

## Build on Ubuntu using Docker
In the root directory run:
```shell
docker compose up --build
docker cp videotagger:/app/build/. ./build/
docker compose down
```

## Build docs
To build the documentation run:
```shell
uv run ./scripts/build_docs.py
```

## Packaging with Velopack
Before packaging make sure to build the project with the `-shipping` preset. Then go to the root directory and run:
```shell
# For help add `--help` flag
uv run ./scripts/vpk_package.py
```

## Third party libraries
- [SDL2](https://github.com/libsdl-org/SDL)
- [ImGui](https://github.com/ocornut/imgui)
- [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo)
- [imgui_toggle](https://github.com/cmdwtf/imgui_toggle)
- [Native File Dialog Extended](https://github.com/btzy/nativefiledialog-extended/tree/master)
- [fmt](https://github.com/fmtlib/fmt)
- [Nlohmann Json](https://github.com/nlohmann/json)
- [utf8.h](https://github.com/sheredom/utf8.h)
- [pybind11](https://github.com/pybind/pybind11)
- [FFmpeg](https://ffmpeg.org/)
- [cpp-httplib](https://github.com/yhirose/cpp-httplib)
- [OpenSSL](https://github.com/openssl/openssl)
- [FreeType](https://gitlab.freedesktop.org/freetype/freetype)
- [OpenCV](https://github.com/opencv/opencv)
- [OpenCV Extra](https://github.com/opencv/opencv_contrib)
- [glad](https://github.com/Dav1dde/glad)
- [onnxruntime](https://github.com/microsoft/onnxruntime)

## License
This software is licensed under the [MIT License](/LICENSE).

This software uses LGPL-licensed libraries from the FFmpeg project.

FFmpeg is an open-source framework licensed under the [GNU Lesser General Public License (LGPL) version 2.1](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html) or later. However, FFmpeg incorporates several optional parts and optimizations that are covered by the [GNU General Public License (GPL) version 2](http://www.gnu.org/licenses/old-licenses/gpl-2.0.html) or later. If those parts get used the GPL applies to all of FFmpeg.

For more information regarding FFmpeg license and legal considerations see https://www.ffmpeg.org/legal.html.
