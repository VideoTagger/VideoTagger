#include "pch.hpp"
#include "bind_project.hpp"
#include <core/app_context.hpp>
#include "proxies.hpp"

void vt::bindings::bind_project(pybind11::module_& module)
{
	namespace py = pybind11;
	py::class_<vt_project>(module, "Project")
	.def_property_readonly("name", [](const vt_project& p) -> std::string
	{
		return p.ref.name;
	})
	.def_property_readonly("videos", [](const vt_project& p) -> std::vector<vt_video>
	{
		std::vector<vt_video> result;
		result.reserve(p.ref.videos.size());
		for (const auto& [k, v] : p.ref.videos)
		{
			result.push_back({ v.path, k, v.width, v.height });
		}
		return result;
	})
	.def_property_readonly("tags", [](const vt_project& p) -> tag_storage&
	{
		return p.ref.tags;
	})
	.def("import_video", [](vt_project& p, const std::string& path) -> std::optional<vt_video>
	{
		auto import_result = p.ref.import_video(path, 0, false).get();
		if (!import_result.success) return std::nullopt;
		auto vid_meta = p.ref.videos.get(import_result.video_id);
		if (vid_meta == nullptr) return std::nullopt;

		return vt_video{ import_result.video_path, import_result.video_id, vid_meta->width, vid_meta->height };
	})
	.def("find_group", [](const vt_project& p, const std::string& name) -> std::optional<video_group>
	{
		auto it = p.ref.video_groups.end();
		for (auto& [id, group] : p.ref.video_groups)
		{
			if (group.display_name == name) return group;
		}
		return std::nullopt;
	})
	.def("add_group", [](vt_project& p, const video_group& group) -> bool
	{
		auto segments = group.segments();
		return p.ref.video_groups.insert({ utils::uuid::get(), group }).second;
	})
	.def_property_readonly("group_queue", [](const vt_project& p) -> video_group_playlist&
	{
		return p.ref.video_group_playlist;
	}, py::return_value_policy::reference_internal);

	module.def("current_project", []() -> std::optional<vt_project>
	{
		return ctx_.current_project.has_value() ? std::optional<vt_project>{ vt_project{ ctx_.current_project.value() } } : std::nullopt;
	});
}
