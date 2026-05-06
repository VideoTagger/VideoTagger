#pragma once
#include <attributes/impl/attribute_instance.hpp>
#include <attributes/region_data.hpp>
#include <events/event.hpp>

namespace vt
{
	class region_event : public event
	{
	public:
		region_event(segment_id segment, impl::attribute_instance& attribute_instance, region_id_t region_id) :
			attribute_instance_{ &attribute_instance }, region_id_{ region_id } {}

	private:
		segment_id segment_;
		impl::attribute_instance* attribute_instance_;
		region_id_t region_id_;

	public:
		segment_id segment() const
		{
			return segment_;
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
