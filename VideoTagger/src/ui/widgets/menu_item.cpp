#include "menu_item.hpp"
#include <pch.hpp>
#include <imgui.h>
#include <ui/widgets/common.hpp>

namespace vt::ui
{
	menu_item::menu_item(const std::string& label_icon, const std::string& label_text, bool enabled) :
		icon_{ label_icon }, label_text_{ label_text }, enabled_{ enabled } {}

	std::string menu_item::label() const
	{
		if (icon_.empty())
		{
			return label_text_;
		}

		return fmt::format("{} {}", icon_, label_text_);
	}

	const std::string& menu_item::icon() const
	{
		return icon_;
	}

	const std::string& menu_item::label_text() const
	{
		return label_text_;
	}

	void menu_item::set_label_text(const std::string& text)
	{
		label_text_ = text;
	}

	void menu_item::set_icon(const std::string& icon)
	{
		icon_ = icon;
	}

	bool menu_item::is_enabled() const
	{
		return enabled_;
	}

	void menu_item::set_enabled(bool enabled)
	{
		enabled_ = enabled;
	}

	menu_button::menu_button(const std::string& label_icon, const std::string& label_text, bool enabled) :
		menu_item{ label_icon, label_text, enabled } {}

	bool menu_button::render()
	{
		if (ImGui::MenuItem(label().c_str(), nullptr, false, is_enabled()))
		{
			on_click();
			return true;
		}
		ui::tooltip(tooltip());

		return false;
	}

	menu_separator::menu_separator() :
		menu_item{ "", "", true}
	{
	}

	menu_separator::menu_separator(const std::string& label) :
		menu_item{ "", label, true}
	{
	}

	bool menu_separator::render()
	{
		const auto& text = label_text();
		if (text.empty())
		{
			ImGui::Separator();
		}
		else
		{
			ImGui::SeparatorText(text.c_str());
		}

		return true;
	}

	submenu::submenu(const std::string& label_icon, const std::string& label_text, bool enabled) :
		menu_item{ label_icon, label_text, enabled }
	{
	}

	bool submenu::render()
	{
		if (ui::begin_menu(label(), is_enabled() and !items_.empty()))
		{
			items_.render();

			ui::end_menu();
			return true;
		}

		return false;
	}

	menu_generic_button::menu_generic_button(const std::string& label_icon, const std::string& label_text, const std::function<void()>& on_click, bool enabled)
		: menu_button{ label_icon, label_text, enabled }, on_click_callback_{ on_click }
	{
	}

	void menu_generic_button::on_click()
	{
		if (on_click_callback_)
		{
			on_click_callback_();
		}
	}
}
