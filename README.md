## Getting started

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

## Third party libraries
- [SDL2](https://github.com/libsdl-org/SDL)
- [ImGui](https://github.com/ocornut/imgui)
- [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo)
- [Native File Dialog Extended](https://github.com/btzy/nativefiledialog-extended/tree/master)
- [Nlohmann Json](https://github.com/nlohmann/json)
- [ffmpeg](https://ffmpeg.org/)