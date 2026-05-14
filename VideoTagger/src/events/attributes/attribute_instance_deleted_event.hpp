#pragma once
#include "attribute_event.hpp"
#include <attributes/impl/attribute_instance.hpp>

namespace vt
{


	class attribute_instance_deleted_event : public attribute_event
	{
	public:
		attribute_instance_deleted_event(const std::string& tag_name, const std::string& attribute_name, const impl::attribute_instance* attribute_instance) :
			attribute_event{ tag_name, attribute_name }, attribute_instance_{ attribute_instance } {}

	private:
		const impl::attribute_instance* attribute_instance_;

	public:
		const impl::attribute_instance* attribute_instance() const
		{
			return attribute_instance_;
		}
	};
}
