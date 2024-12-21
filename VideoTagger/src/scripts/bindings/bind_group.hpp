#pragma once
#include <pybind11/embed.h>

namespace vt::bindings
{
	extern void bind_group(pybind11::module_& module);
}
