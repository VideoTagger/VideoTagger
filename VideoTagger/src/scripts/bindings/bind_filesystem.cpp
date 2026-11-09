#include "pch.hpp"
#include "bind_filesystem.hpp"
#include <utils/filesystem.hpp>

extern void vt::bindings::bind_filesystem(pybind11::module_& mod)
{
	namespace py = pybind11;

	auto submod = mod.def_submodule("fs", "Bindings for filesystem-related functionality");
	submod.def("open_in_explorer", [](const std::string& path)
	{
		vt::utils::filesystem::open_in_explorer(path);
	}, py::arg("path"));

	submod.def("open_file_in_explorer", [](const std::string& path)
	{
		vt::utils::filesystem::open_file_in_explorer(path);
	}, py::arg("path"));
}
