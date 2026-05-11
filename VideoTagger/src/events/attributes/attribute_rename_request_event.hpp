#pragma once
#include "attribute_event.hpp"

namespace vt
{
	class attribute_rename_request_event : public attribute_event
	{
	public:
		attribute_rename_request_event(const std::string& tag_name, const std::string& attribute_name, const std::string& new_name) :
			attribute_event{ tag_name, attribute_name }, new_name_{ new_name } {}

	private:
		std::string new_name_;

	public:
		const std::string& new_name() const
		{
			return new_name_;
		}
	};
}
