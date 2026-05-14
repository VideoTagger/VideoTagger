#pragma once
#include <events/event.hpp>
#include <string>

namespace vt
{
	class attribute_event : public event
	{
	public:
		attribute_event(const std::string& tag_name, const std::string& attribute_name) :
			tag_name_{ tag_name }, attribute_name_{ attribute_name } {}

	private:
		std::string tag_name_;
		std::string attribute_name_;

	public:
		const std::string& tag_name() const
		{
			return tag_name_;
		}
		
		const std::string& attribute_name() const
		{
			return attribute_name_;
		}
	};
}
