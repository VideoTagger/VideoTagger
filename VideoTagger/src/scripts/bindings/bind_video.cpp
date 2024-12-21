#include "pch.hpp"
#include "bind_video.hpp"
#include <core/app_context.hpp>
#include "proxies.hpp"

void vt::bindings::bind_video(pybind11::module_& module)
{
	namespace py = pybind11;

	py::class_<vt_video>(module, "Video")
	.def_property_readonly("id", [](const vt_video& vid) -> video_id_t
	{
		return vid.id;
	})
	.def_property_readonly("path", [](const vt_video& vid) -> std::string
	{
		return vid.path.string();
	})
	.def_property_readonly("size", [](const vt_video& vid) -> std::pair<int, int>
	{
		return std::make_pair(vid.width, vid.height);
	});
}
