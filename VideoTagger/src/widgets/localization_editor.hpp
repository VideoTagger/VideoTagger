#pragma once
#include <string>
#include <vector>
#include <memory>
#include <core/localization/lang_pack.hpp>

namespace vt::widgets
{
	struct localization_editor
	{
		void render(bool& is_open, std::vector<std::shared_ptr<lang_pack>>& langs);

		static std::string window_name();
	};
}
