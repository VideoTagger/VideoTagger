#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <limits>
#include <functional>

#include <tags/tag_storage.hpp>

namespace vt::widgets
{
	bool tag_manager(tag_storage& tags, tag_storage::iterator& selected_entry);
}
