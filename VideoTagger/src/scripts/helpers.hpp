#pragma once
#include <map>
#include <pybind11/embed.h>
#include <pybind11/stl.h>

namespace vt::bindings::helpers
{
	namespace py = pybind11;

	template<typename key_t, typename value_t>
	static std::map<key_t, value_t> to_map(py::dict d)
	{
		std::map<key_t, value_t> result;
		for (std::pair<py::handle, py::handle> item : d)
		{
			auto key = item.first.cast<key_t>();
			result[key] = item.second.cast<value_t>();
		}
		return result;
	}
}
