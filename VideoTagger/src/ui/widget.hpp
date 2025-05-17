#pragma once
#include <string>

namespace vt::ui
{
	struct widget
	{
		bool render_with_label(const std::string& label, bool sameline = false);
		virtual bool render() = 0;
	};
}
