#pragma once
#include "attribute_event.hpp"
#include <attributes/impl/attribute_instance.hpp>

namespace vt
{


	class attribute_instance_deleted_event : public event
	{
	public:
		attribute_instance_deleted_event(const std::string& tag_name, segment_id segment, video_id_t video_id, const impl::attribute_instance* attribute_instance) :
			tag_name_{ tag_name }, segment_{ segment }, video_id_{ video_id }, attribute_instance_{ attribute_instance } {}

	private:
		std::string tag_name_;
		segment_id segment_;
		video_id_t video_id_;
		const impl::attribute_instance* attribute_instance_;

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

		const impl::attribute_instance* attribute_instance() const
		{
			return attribute_instance_;
		}
	};
}
