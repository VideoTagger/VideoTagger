## Getting started

### Visual Studio (Windows)
#### Generating project files
In order to generate project files run `scripts/win-gen-projects.cmd`.
#### Building
To build the projects, open the `.sln` file in Visual Studio and build the projects with chosen configuration.


### Makefile (Linux)
In order to generate project files run `scripts/linux-gen-projects.sh`.
#### Requirements
- `SDL2` version 2.0.17 or later

#### Building
To build the projects run `./make config=<config>`, where `<config>` is your desired configuration. Available configurations:
- `debug_x86_64`
- `release_x86_64`