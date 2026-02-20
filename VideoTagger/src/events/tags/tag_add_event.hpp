#pragma once
#include "tag_event.hpp"

namespace vt
{
	struct tag_add_event : public tag_event
	{
		tag_add_event(tag_storage& tag_storage, const std::string& tag_name, tag_validate_result validate_result) :
			tag_event(tag_storage, tag_name), validate_result_{ validate_result } {}

	private:
		tag_validate_result validate_result_;

	public:
		constexpr tag_validate_result validate_result() const
		{
			return validate_result_;
		}
	};
}
