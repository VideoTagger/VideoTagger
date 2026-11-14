#pragma once
#include <pybind11/embed.h>

namespace vt::bindings
{
	extern void bind_system(pybind11::module_& mod);
}
