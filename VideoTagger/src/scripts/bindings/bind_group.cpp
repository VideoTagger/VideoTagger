#include "pch.hpp"
#include "bind_group.hpp"
#include <core/app_context.hpp>
#include "proxies.hpp"

void vt::bindings::bind_group(pybind11::module_& module)
{
	namespace py = pybind11;
	
	py::class_<video_group::video_info>(module, "VideoInfo")
	.def_readonly("id", &video_group::video_info::id)
	.def_property_readonly("offset", [](const video_group::video_info& vi) -> timestamp
	{
		return timestamp{ std::chrono::duration_cast<std::chrono::milliseconds>(vi.offset).count() };
	});

	py::class_<video_group>(module, "VideoGroup")
	.def(py::init([](const std::string& name) -> video_group
	{
		return video_group{ name, {} };
	}))
	.def_readwrite("name", &video_group::display_name)
	.def_property_readonly("video_infos", [](const video_group& group) -> std::vector<video_group::video_info>
	{
		return group.videos();
	})
	.def("add_video", [](video_group& group, const vt_video& vid, timestamp offset)
	{
		group.insert(video_group::video_info{ vid.id, std::chrono::nanoseconds(offset.total_milliseconds) });
	})
	.def("add_timestamp", [](video_group& group, tag& t, timestamp start) -> std::optional<vt_tag_segment>
	{
		auto it = group.segments()[t.name].insert(start);
		if (it.second)
		{
			return vt_tag_segment{ (tag_segment&)(*it.first), t };
		}
		return std::nullopt;
	})
	.def("add_segment", [](video_group& group, tag& t, timestamp start, timestamp end) -> std::optional<vt_tag_segment>
	{
		auto it = group.segments()[t.name].insert(start, end);
		if (it.second)
		{
			return vt_tag_segment{ (tag_segment&)(*it.first), t };
		}
		return std::nullopt;
	})
	.def("get_segments", [](video_group& group, tag& t) -> std::vector<vt_tag_segment>
	{
		std::vector<vt_tag_segment> result;
		auto& segm = group.segments();
		auto segments_it = segm.find(t.name);
		if (segments_it != segm.end())
		{
			for (const auto& segment : segments_it->second)
			{
				result.push_back(vt_tag_segment{ (tag_segment&)(segment), t });
			}
		}
		return result;
	});

	py::class_<video_group_playlist>(module, "GroupQueue")
	.def("add_group", [](video_group_playlist& p, const video_group& g) -> bool
	{
		auto& groups = ctx_.current_project->video_groups;
		auto it = std::find_if(groups.begin(), groups.end(), [&g](const std::pair<video_group_id_t, video_group>& group)
		{
			return group.second.display_name == g.display_name;
		});
		if (it == groups.end()) return false;
		p.insert(p.end(), it->first);
		return true;
	})
	.def("current_group", [](video_group_playlist& p) -> std::optional<video_group*>
	{
		auto it = p.current();
		auto& groups = ctx_.current_project->video_groups;
		if (it == p.end()) return std::nullopt;
		auto git = groups.find(*it);
		return git != groups.end()  ? std::optional{ &git->second } : std::nullopt;
	}, py::return_value_policy::reference_internal)
	.def_property_readonly("groups", [](const video_group_playlist& p) -> std::vector<video_group*>
	{
		std::vector<video_group*> result;
		auto& groups = ctx_.current_project->video_groups;
		for (auto group_id : p)
		{
			const auto& group_it = groups.find(group_id);
			if (group_it == groups.end()) continue;
			result.push_back(&group_it->second);
		}
		return result;
	}, py::return_value_policy::reference_internal);
}
