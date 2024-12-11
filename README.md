# VideoTagger

![VideoTagger Preview](/resources/images/preview.png?raw=true "VideoTagger")

## Getting started
To download the repository use `git clone --recursive https://github.com/VideoTagger/VideoTagger`.

If the repository was cloned non-recursively use `git submodule update --init` to clone the submodules.


## Setting up the project
### Visual Studio (Windows)
Visual Studio 2019 or 2022 is recommended
#### Generating project files
In order to generate project files run `scripts/win-gen-projects.cmd`.
#### Building
To build the projects, open the `.sln` file in Visual Studio and build the projects with chosen configuration.

### Makefile (Linux)
In order to generate project files run `scripts/linux-gen-projects.sh`.
#### Requirements
- `SDL2` version 2.0.17 or later
#### Building
To build the projects run `make config=<config>`, where `<config>` is your desired configuration. Available configurations:
- `debug_x86_64`
- `release_x86_64`
- `shipping_x86_64`

### Xcode (MacOS)
In order to generate project files run `scripts/macos-gen-projects.sh`.
#### Requirements
- `SDL2` version 2.0.17 or later
#### Building
To build the projects, open the Xcode file and build the projects with chosen configuration.


## Third party libraries
- [SDL2](https://github.com/libsdl-org/SDL)
- [ImGui](https://github.com/ocornut/imgui)
- [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo)
- [Native File Dialog Extended](https://github.com/btzy/nativefiledialog-extended/tree/master)
- [fmt](https://github.com/fmtlib/fmt)
- [Nlohmann Json](https://github.com/nlohmann/json)
- [utf8.h](https://github.com/sheredom/utf8.h)
- [pybind11](https://github.com/pybind/pybind11)
- [ffmpeg](https://ffmpeg.org/)
