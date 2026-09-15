#pragma once
#include "tag_event.hpp"

namespace vt
{
	struct tag_add_request_event : tag_event
	{
		tag_add_request_event(tag_storage& tag_storage, const std::string& tag_name, uint32_t color) :
			tag_event(tag_storage, tag_name), color_{ color } {}

	private:
		uint32_t color_;

	public:
		constexpr uint32_t color() const
		{
			return color_;
		}
	};
}
