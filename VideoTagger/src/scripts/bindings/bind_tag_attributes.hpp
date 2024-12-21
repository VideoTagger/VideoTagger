#pragma once
#include <pybind11/embed.h>

namespace vt::bindings
{
	extern void bind_tag_attributes(pybind11::module_& module);
}
