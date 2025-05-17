#pragma once
#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <future>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <numeric>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>
#include <execution>
#include <unordered_set>
#include <unordered_map>

#include <SDL.h>
#include <SDL_opengl.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>

#define FMT_HEADER_ONLY
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <nfd.hpp>
#include <utf8.h>

#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <pybind11/operators.h>
