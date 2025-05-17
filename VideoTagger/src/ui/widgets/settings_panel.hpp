#pragma once
#include <string>
#include "widget_list.hpp"

namespace vt::ui
{
	struct settings_panel : public widget_list
	{
		settings_panel() = default;

		settings_panel& add_toggle(const std::string& label, const std::string& description, bool& value, const std::function<void(bool value)>& on_toggle = nullptr);
		settings_panel& add_label_spacer(const std::string& label);

		template<typename widget_type, typename... arguments>
		constexpr settings_panel& add(arguments&&... args)
		{
			widget_list::add<widget_type>(std::forward<arguments>(args)...);
			return *this;
		}

		constexpr settings_panel& add_raw(const std::function<bool()>& render_callback)
		{
			return add<raw_widget>(render_callback);
		}
	};
}
