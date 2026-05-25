#pragma once
#include <events/event.hpp>
#include <string>
#include <core/types.hpp>
#include <attributes/impl/attribute_instance.hpp>

namespace vt
{
	template<typename shape_type>
	class region_insert_request_event : public event
	{
	public:
		region_insert_request_event(const std::string& tag_name, segment_id segment, video_id_t video_id, impl::shape_attribute_instance* attribute_instance, timestamp ts, const shape_type& shape) :
			tag_name_{ tag_name }, segment_{ segment }, video_id_{ video_id }, attribute_instance_{ attribute_instance }, ts_{ ts }, shape_{ shape } {}

	private:
		std::string tag_name_;
		segment_id segment_;
		video_id_t video_id_;
		impl::shape_attribute_instance* attribute_instance_;
		timestamp ts_;
		shape_type shape_;

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

		impl::attribute_instance* attribute_instance() const
		{
			return attribute_instance_;
		}

		const timestamp& timestamp() const
		{
			return ts_;
		}

		const shape_type& shape() const
		{
			return shape_;
		}
	};
}
