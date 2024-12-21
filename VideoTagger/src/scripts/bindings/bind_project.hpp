#pragma once
#include <pybind11/embed.h>

namespace vt::bindings
{
	extern void bind_project(pybind11::module_& module);
}
