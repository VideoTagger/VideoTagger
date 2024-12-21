#pragma once
#include <core/project.hpp>
#include <core/types.hpp>
#include <video/video_pool.hpp>
#include <tags/tag.hpp>

namespace vt::bindings
{
	struct vt_project
	{
		project& ref;
	};

	struct vt_video
	{
		std::filesystem::path path;
		video_id_t id{};
		int width{};
		int height{};
	};

	struct vt_video_group
	{
		video_group_id_t id{};
		video_group& ref;
	};

	struct vt_tag_segment
	{
		tag_segment& ref;
		tag& tag_ref;
	};
}
