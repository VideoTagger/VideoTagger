#pragma once
#include <core/project.hpp>
#include <core/types.hpp>
#include <video/video_pool.hpp>
#include <tags/tag.hpp>
#include <attributes/core/shape_attribute_instance.hpp>

namespace vt::bindings
{
	struct vt_tag
	{
		tag_storage* storage{};
		tag* ptr{};
	};

	struct vt_segment
	{
		vt_tag tag;
		tag_timeline* timeline{};
		segment_id id = invalid_segment_id;
		const tag_segment* ptr{};
	};

	struct vt_attribute
	{
		vt_tag tag;
		impl::attribute* ptr{};
	};

	struct vt_attribute_instance
	{
		vt_segment segment;
		impl::attribute_instance* ptr{};
	};

	struct vt_project
	{
		project* ptr{};
	};

	struct vt_video
	{
		std::filesystem::path path;
		video_id_t id{};
		int width{};
		int height{};

		constexpr bool operator==(const vt_video& other) const
		{
			return id == other.id;
		}
	};

	struct vt_video_group
	{
		video_group_id_t id{};
		video_group* ptr{};
	};

	template<typename shape_type>
	struct vt_shape_attribute_instance
	{
		vt_segment segment;
		shape_attribute_instance<shape_type>* ptr{};
	};

	inline std::optional<vt_video> make_vt_video(project& project, video_id_t id)
	{
		auto video_ptr = project.videos.get(id);
		if (video_ptr == nullptr) return std::nullopt;

		return vt_video{ video_ptr->file_path(), id, video_ptr->width(), video_ptr->height() };
	}
}

template<>
struct std::hash<vt::bindings::vt_video>
{
	std::size_t operator()(const vt::bindings::vt_video& v) const noexcept
	{
		return std::hash<vt::video_id_t>{}(v.id);
	}
};
