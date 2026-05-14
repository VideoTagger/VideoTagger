#pragma once
#include <attributes/impl/attribute_instance.hpp>
#include <core/types.hpp>
#include <events/event.hpp>

namespace vt
{
	class region_event : public event
	{
	public:
		region_event(const std::string& tag_name, segment_id segment, video_id_t video_id, impl::attribute_instance& attribute_instance, region_id_t region_id) :
			tag_name_{ tag_name }, segment_{ segment }, video_id_{ video_id }, attribute_instance_{ &attribute_instance }, region_id_{ region_id } {}

	private:
		std::string tag_name_;
		segment_id segment_;
		video_id_t video_id_;
		impl::attribute_instance* attribute_instance_;
		region_id_t region_id_;

	public:
		const std::string& tag_name() const
		{
			return tag_name_;
		}

		segment_id segment() const
		{
			return segment_;
		}

		video_id_t video_id() const
		{
			return video_id_;
		}

		impl::attribute_instance& attribute_instance() const
		{
			return *attribute_instance_;
		}

		region_id_t region_id() const
		{
			return region_id_;
		}
	};
}
