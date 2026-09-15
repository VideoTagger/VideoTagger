#pragma once
#include <attributes/impl/attribute_instance.hpp>
#include <utils/timestamp_span.hpp>
#include <video/video_stream.hpp>
#include <optional>
#include <tasks/cancellation_token.hpp>
#include <attributes/impl/region_tracker.hpp>
#include <typeinfo>

namespace vt::impl
{
	class shape_attribute_instance : public attribute_instance
	{
	public:
		shape_attribute_instance(attribute* attr) : attribute_instance{ attr } {}

		virtual bool region_exists(region_id_t id) const = 0;
		
		virtual const std::string& region_name(region_id_t region_id) const = 0;

		virtual std::vector<region_id_t> region_ids() const = 0;
		virtual std::vector<timestamp> keyframe_timestamps(region_id_t region_id) const = 0;
		virtual std::optional<timestamp> first_keyframe_timestamp(region_id_t region_id) const = 0;
		virtual std::optional<timestamp> last_keyframe_timestamp(region_id_t region_id) const = 0;
		virtual bool is_keyframe(region_id_t region_id, timestamp ts) const = 0;

		virtual const std::type_info& shape_type_info() const = 0;

		virtual std::unique_ptr<impl::region_tracker> new_region_tracker() = 0;
	};
}
