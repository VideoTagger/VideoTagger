#include "pch.hpp"
#include "bind_color.hpp"
#include <utils/color.hpp>
#include <utils/random.hpp>

extern void vt::bindings::bind_color(pybind11::module_& mod)
{
	namespace py = pybind11;

	auto submod = mod.def_submodule("color", "Bindings for color-related functionality");
	submod.def("to_abgr", [](uint32_t color)
	{
		return vt::utils::color::to_abgr(color);
	}, py::arg("color"));

	submod.def("random_color", []() -> uint32_t
	{
		ImVec4 color{ 0, 0, 0, 1 };

		auto hue = utils::random::get_from_zero<float>();
		auto value = utils::random::get<float>(0.5f, 1.0f);
		ImGui::ColorConvertHSVtoRGB(hue, 0.75f, value, color.x, color.y, color.z);
		return ImGui::ColorConvertFloat4ToU32(color);
	});
}
