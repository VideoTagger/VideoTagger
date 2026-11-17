#include "pch.hpp"
#include "bind_segment.hpp"
#include <pybind11/operators.h>
#include "proxies.hpp"
#include <core/app_context.hpp>

void vt::bindings::bind_segment(pybind11::module_& mod)
{
	namespace py = pybind11;

	py::class_<vt_segment>(mod, "Segment")
	.def_property_readonly("attribute_instances", [](vt_segment& s) -> std::unordered_map<vt_video, std::vector<vt_attribute_instance>>
	{
		std::unordered_map<vt_video, std::vector<vt_attribute_instance>> result;

		auto& instances = s.timeline->segment_attribute_instances(s.id);
		for (auto& [video_id, attr_instances] : instances)
		{
			auto video = make_vt_video(*ctx_.current_project, video_id);
			if (!video.has_value()) continue;

			auto& result_attr_instances = result[*video];
			for (auto& instance : attr_instances)
			{
				result_attr_instances.emplace_back(vt_attribute_instance{ s, instance.get() });
			}
		}

		return result;
	})
	.def("get_attribute_instance", [](vt_segment& s, const vt_video& vid, const std::string& name) -> std::optional<vt_attribute_instance>
	{
		auto& video_instances = s.timeline->segment_attribute_instances(s.id);
		auto video_it = video_instances.find(vid.id);
		if (video_it == video_instances.end()) return std::nullopt;

		auto& attr_instances = video_it->second;
		auto instance_it = std::find_if(attr_instances.begin(), attr_instances.end(), [&name](const auto& ptr)
		{
			return ptr->attribute_name() == name;
		});

		if (instance_it == attr_instances.end()) return std::nullopt;

		return vt_attribute_instance{ s, instance_it->get() };
	}, py::arg("video"), py::arg("name"), py::return_value_policy::reference_internal)
	.def_property_readonly("tag", [](const vt_segment& s) -> vt_tag
	{
		return s.tag;
	})
	.def_property_readonly("id", [](const vt_segment& s) -> segment_id
	{
		return s.id;
	})
	.def_property_readonly("start", [](const vt_segment& s) -> timestamp
	{
		return s.ptr->start;
	})
	.def_property_readonly("end", [](const vt_segment& s) -> timestamp
	{
		return s.ptr->end;
	});
}
