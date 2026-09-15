#pragma once
#include "tag_event.hpp"

namespace vt
{
	struct tag_rename_request_event : public tag_event
	{
		tag_rename_request_event(tag_storage& tag_storage, const std::string& current_name, const std::string& new_name) :
			tag_event(tag_storage, current_name), new_name_(new_name) {
		}

	private:
		std::string new_name_;

	public:
		constexpr const std::string& new_name() const
		{
			return new_name_;
		}
	};
}
