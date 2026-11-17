#include "pch.hpp"
#include "bind_group.hpp"
#include <core/app_context.hpp>
#include "proxies.hpp"

void vt::bindings::bind_group(pybind11::module_& mod)
{
	namespace py = pybind11;
	
	py::class_<video_group::video_info>(mod, "VideoInfo")
	.def_readonly("id", &video_group::video_info::id)
	.def_property_readonly("offset", [](const video_group::video_info& vi) -> timestamp
	{
		return timestamp{ vi.offset };
	});

	py::class_<vt_video_group>(mod, "VideoGroup")
	.def_property_readonly("name", [](const vt_video_group& group)
	{
		return group.ptr->display_name;
	})
	.def_property_readonly("video_infos", [](const vt_video_group& group) -> std::vector<video_group::video_info>
	{
		return group.ptr->videos();
	})
	.def("add_video", [](vt_video_group& group, const vt_video& vid, timestamp offset)
	{
		//TODO: use event

		group.ptr->insert(video_group::video_info{ vid.id, offset.total_nanoseconds });
	}, py::arg("video"), py::arg("offset"))
	.def("add_timestamp", [](vt_video_group& group, vt_tag t, timestamp start) -> std::optional<vt_segment>
	{
		auto& segments = group.ptr->segments()[t.ptr->name];
		auto insert_result = segments.insert(start);
		if (insert_result.inserted())
		{
			auto inserted_id = insert_result.inserted_segment();
			auto& segment = segments.at(inserted_id);
			return vt_segment{ t, &segments, inserted_id, &segment };
		}
		return std::nullopt;
	}, py::arg("tag"), py::arg("start"))
	.def("add_segment", [](vt_video_group& group, vt_tag& t, timestamp start, timestamp end) -> std::optional<vt_segment>
	{
		auto& segments = group.ptr->segments()[t.ptr->name];
		auto insert_result = segments.insert(start, end);
		if (insert_result.inserted())
		{
			auto inserted_id = insert_result.inserted_segment();
			auto& segment = segments.at(inserted_id);
			return vt_segment{ t, &segments, inserted_id, &segment };
		}
		return std::nullopt;
	}, py::arg("tag"), py::arg("start"), py::arg("end"))
	.def("get_segments", [](vt_video_group& group, vt_tag& t) -> std::vector<vt_segment>
	{
		std::vector<vt_segment> result;
		auto& storage = group.ptr->segments();
		auto segments_it = storage.find(t.ptr->name);
		if (segments_it != storage.end())
		{
			auto& segments = segments_it->second;
			for (const auto& [id, segment] : segments)
			{
				result.push_back(vt_segment{ t, &segments, id, &segment });
			}
		}
		return result;
	}, py::arg("tag"));

	py::class_<video_group_playlist>(mod, "GroupQueue")
	.def("add_group", [](video_group_playlist& p, const vt_video_group& g) -> bool
	{
		auto& groups = ctx_.current_project->video_groups;
		if (groups.count(g.id) == 0) return false;

		p.insert(p.end(), g.id);
		return true;
	}, py::arg("group"))
	.def("current_group", [](video_group_playlist& p) -> std::optional<vt_video_group>
	{
		auto it = p.current();
		auto& groups = ctx_.current_project->video_groups;
		if (it == p.end()) return std::nullopt;
		auto git = groups.find(*it);
		if (git == groups.end()) return std::nullopt;

		return vt_video_group{ git->first, &git->second };
	})
	.def_property_readonly("groups", [](const video_group_playlist& p) -> std::vector<vt_video_group>
	{
		std::vector<vt_video_group> result;
		auto& groups = ctx_.current_project->video_groups;
		for (auto group_id : p)
		{
			const auto& git = groups.find(group_id);
			if (git == groups.end()) continue;

			result.emplace_back(vt_video_group{ git->first, &git->second });
		}
		return result;
	});
}
