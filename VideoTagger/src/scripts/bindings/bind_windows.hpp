#pragma once
#include <pybind11/embed.h>

namespace vt::bindings
{
	extern void bind_windows(pybind11::module_& mod);
}
