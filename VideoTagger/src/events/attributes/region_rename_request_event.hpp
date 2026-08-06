#pragma once
#include <events/attributes/region_event.hpp>

namespace vt
{
	class region_rename_request_event : public region_event
	{
	public:
		region_rename_request_event(const std::string& tag_name, segment_id segment, video_id_t video_id, impl::shape_attribute_instance* attribute_instance, region_id_t region_id, const std::string& new_name) :
			region_event{ tag_name, segment, video_id, attribute_instance, region_id }, new_name_{ new_name } {}

	private:
		std::string new_name_;

	public:
		const std::string& new_name() const
		{
			return new_name_;
		}
	};
}
