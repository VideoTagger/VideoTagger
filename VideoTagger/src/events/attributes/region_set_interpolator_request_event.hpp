#pragma once
#include <events/attributes/region_event.hpp>

namespace vt
{
	class region_set_interpolator_request_event : public region_event
	{
	public:
		region_set_interpolator_request_event(const std::string& tag_name, segment_id segment, video_id_t video_id, impl::shape_attribute_instance* attribute_instance, region_id_t region_id, const std::string& interpolator_name) :
			region_event{ tag_name, segment, video_id, attribute_instance, region_id }, interpolator_name_{ interpolator_name } {}

	private:
		std::string interpolator_name_;

	public:
		const std::string& interpolator_name() const
		{
			return interpolator_name_;
		}
	};
}
