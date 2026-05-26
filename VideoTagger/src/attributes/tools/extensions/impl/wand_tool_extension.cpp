#include "wand_tool_extension.hpp"

namespace vt::ui::impl
{
	wand_tool_extension::wand_tool_extension(const std::string& name) : name_(name) {}

	const std::string& wand_tool_extension::name() const
	{
		return name_;
	}
}
