#include "pch.hpp"
#include "bind_project.hpp"
#include <core/app_context.hpp>
#include "proxies.hpp"
#include <video/local_video_resource.hpp>
#include <video/local_video_importer.hpp>

void vt::bindings::bind_project(pybind11::module_& mod)
{
	namespace py = pybind11;
	py::class_<vt_project>(mod, "Project")
	.def_property_readonly("name", [](const vt_project& p) -> std::string
	{
		return p.ptr->name;
	})
	.def_property_readonly("videos", [](const vt_project& p) -> std::vector<vt_video>
	{
		std::vector<vt_video> result;
		result.reserve(p.ptr->videos.size());
		for (const auto& [k, v] : p.ptr->videos)
		{
			result.push_back({ v->file_path(), k, v->width(), v->height() });
		}
		return result;
	})
	.def_property_readonly("tags", [](const vt_project& p) -> tag_storage&
	{
		return p.ptr->tags;
	})
	.def("import_video", [](vt_project& p, const std::string& path) -> std::optional<vt_video>
	{
		auto vid_resource_id = video_importer::generate_video_id();
		auto vid_resource = ctx_.get_video_importer<local_video_importer>().import_video(vid_resource_id, std::filesystem::path(path));
		if (vid_resource == nullptr) return std::nullopt;

		if (!p.ptr->import_video(std::move(vid_resource), std::nullopt)) return std::nullopt;

		auto vid = p.ptr->videos.get<local_video_resource>(vid_resource_id);
		if (vid == nullptr)
		{
			return std::nullopt;
		}

		return vt_video{ vid->file_path(), vid->id(), vid->width(), vid->height() };
	}, py::arg("path"))
	.def("get_video", [](vt_project& p, video_id_t id) -> std::optional<vt_video>
	{
		return make_vt_video(*p.ptr, id);
	}, py::arg("id"))
	.def("remove_video", [](vt_project& p, vt_video& v) -> bool
	{
		//TODO: Add event for removing videos and use it here

		auto it = std::find_if(p.ptr->videos.begin(), p.ptr->videos.end(), [&v](const decltype(p.ptr->videos)::iterator::value_type& vit)
		{
			return vit.first == v.id;
		});
		if (it == p.ptr->videos.end()) return false;
		p.ptr->remove_video(v.id);
		return true;
	}, py::arg("video"))
	.def("find_group", [](const vt_project& p, const std::string& name) -> std::optional<vt_video_group>
	{
		auto it = p.ptr->video_groups.end();
		for (auto& [id, group] : p.ptr->video_groups)
		{
			if (group.display_name == name) return vt_video_group{ id, &group };
		}
		return std::nullopt;
	}, py::return_value_policy::reference_internal)
	.def("add_group", [](vt_project& p, const std::string& name) -> std::optional<vt_video_group>
	{
		//TODO: Add event for adding groups and use it here

		auto [it, inserted] = p.ptr->video_groups.try_emplace(utils::random::get_uuid());
		if (!inserted) return std::nullopt;

		auto id = it->first;
		auto& group = it->second;
		// No idea if group names are unique or not
		group.display_name = name;

		return vt_video_group{ id, &group };
	})
	.def_property_readonly("group_queue", [](const vt_project& p) -> video_group_playlist&
	{
		return p.ptr->video_group_playlist;
	}, py::return_value_policy::reference_internal);

	mod.def("current_project", []() -> std::optional<vt_project>
	{
		return ctx_.current_project.has_value() ? std::optional<vt_project>{ vt_project{ &ctx_.current_project.value() } } : std::nullopt;
	});
}
