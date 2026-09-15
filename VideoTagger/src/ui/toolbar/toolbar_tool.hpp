#pragma once
#include <string>
#include <ui/widgets/widget_list.hpp>
#include <ui/widgets/button_bar.hpp>
#include <ui/toolbar/toolbar_tool_extension.hpp>

namespace vt::ui
{
	struct toolbar_tool : public toolbar_tool_extension
	{
	public:
		toolbar_tool() = default;

	private:
		bool has_body_ = false;

	public:
		constexpr void set_has_body(bool value)
		{
			has_body_ = value;
		}

		constexpr bool has_body() const
		{
			return has_body_;
		}

		///@return The name to be displayed in the tool's popup as a combo item.
		virtual std::string display_name() const
		{
			return "unknown-tool";
		}

		///@brief Renders the body of the tool's popup. Only relevant if has_body() returns true.
		virtual void render_popup_body(ui::widget_list& widgets, ui::button_bar<int>& button_bar) {}
	};
}
