#pragma once
#include "attribute_event.hpp"

namespace vt
{
	class attribute_deleted_event : public attribute_event
	{
	public:
		attribute_deleted_event(const std::string& tag_name, const std::string& attribute_name) :
			attribute_event{ tag_name, attribute_name } {}
	};
}
