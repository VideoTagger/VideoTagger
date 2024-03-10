#pragma once
#include <string>
#include <vector>

#include <tags/tag_storage.hpp>

namespace vt::widgets
{
	extern bool tag_menu(tag_storage& tags, std::vector<std::string>& visible_tags);
}
