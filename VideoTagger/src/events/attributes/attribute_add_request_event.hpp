#pragma once
#include "attribute_event.hpp"

namespace vt
{
	class attribute_add_request_event : public attribute_event
	{
	public:
		attribute_add_request_event(const std::string& tag_name, const std::string& attribute_name, const std::string& type_name) :
			attribute_event{ tag_name, attribute_name }, type_name_{ type_name } {}

	private:
		std::string type_name_;

	public:
		const std::string& type_name() const
		{
			return type_name_;
		}
	};
}
