#include "pch.hpp"
#include "bind_project.hpp"
#include <core/app_context.hpp>
#include "proxies.hpp"
#include <video/local_video_resource.hpp>
#include <video/local_video_importer.hpp>

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
			const auto& metadata = v->metadata();
			result.push_back({ v->file_path(), k, metadata.width.value_or(0), metadata.height.value_or(0) });
		}
		return result;
	})
	.def_property_readonly("tags", [](const vt_project& p) -> tag_storage&
	{
		return p.ref.tags;
	})
	.def("import_video", [](vt_project& p, const std::string& path) -> std::optional<vt_video>
	{
		auto vid_resource_id = video_importer::generate_video_id();
		auto vid_resource = ctx_.get_video_importer<local_video_importer>().import_video(vid_resource_id, std::filesystem::path(path));
		if (vid_resource == nullptr) return std::nullopt;

		if (!p.ref.import_video(std::move(vid_resource), std::nullopt)) return std::nullopt;

		auto& vid = p.ref.videos.get<local_video_resource>(vid_resource_id);

		const auto& metadata = vid.metadata();
		return vt_video{ vid.file_path(), vid.id(), metadata.width.value_or(0), metadata.height.value_or(0) };
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
