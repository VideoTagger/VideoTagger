#pragma once
#include <string>
#include <imgui.h>
#include <core/types.hpp>
#include <ui/widgets/widget_list.hpp>
#include <ui/widgets/button_bar.hpp>

namespace vt::ui
{
	struct toolbar_tool
	{
	public:
		toolbar_tool() = default;

	private:
		bool has_body_ = false;
		uint32_t property_columns_ = 0;

	public:
		constexpr void set_has_body(bool value)
		{
			has_body_ = value;
		}

		constexpr bool has_body() const
		{
			return has_body_;
		}

		constexpr void set_property_column_count(uint32_t count)
		{
			property_columns_ = count;
		}

		constexpr uint32_t property_column_count() const
		{
			return property_columns_;
		}

		///@return The name to be displayed in the tool's popup as a combo item.
		virtual std::string display_name() const
		{
			return "unknown-tool";
		}

		///@brief Called when the tool is activated
		virtual void on_activate() {}
		///@brief Called when the tool is deactivated
		virtual void on_deativate() {}

		///@brief Called when the "Done" button is pressed in the tool's popup.
		virtual void on_done() {}
		///@brief Called when any button other than "Done" is pressed in the tool's popup. The button ID is passed as an argument.
		virtual void on_button_click(int id) {}

		virtual void render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size) {}
		///@brief Renders the body of the tool's popup. Only relevant if has_body() returns true.
		virtual void render_popup_body(ui::widget_list& widgets, ui::button_bar<int>& button_bar) {}

		/**
		 * @brief Renders the properties of the tool in the properties window.
		 * 
		 * Only relevant if property_column_count() returns a value greater than 0.
		 * Each tool should call ImGui::TableNextColumn() before rendering the content of each column.
		 * The number of columns is determined by the value returned by property_column_count().
		 */
		virtual void render_properties() {}
	};
}
