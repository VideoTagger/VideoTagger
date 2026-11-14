#include "pch.hpp"
#include "bind_color.hpp"
#include <utils/color.hpp>

extern void vt::bindings::bind_color(pybind11::module_& mod)
{
	namespace py = pybind11;

	auto submod = mod.def_submodule("color", "Bindings for color-related functionality");
	submod.def("to_abgr", [](uint32_t color)
	{
		return vt::utils::color::to_abgr(color);
	}, py::arg("color"));
}
