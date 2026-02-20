#pragma once
#include "tag_event.hpp"

namespace vt
{
	struct tag_delete_event : public tag_event
	{
		tag_delete_event(tag_storage& tag_storage, const std::string& tag_name) : tag_event(tag_storage, tag_name) {}
	};
}
