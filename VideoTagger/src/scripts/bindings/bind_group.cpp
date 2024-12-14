#include "pch.hpp"
#include "bind_group.hpp"
#include <core/app_context.hpp>
#include "proxies.hpp"

void vt::bindings::bind_group(pybind11::module_& module)
{
	namespace py = pybind11;
	
	py::class_<video_group>(module, "VideoGroup")
	.def(py::init([](const std::string& name) -> video_group
	{
		return video_group{ name, {} };
	}))
	.def_readwrite("name", &video_group::display_name)
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
	});
}
