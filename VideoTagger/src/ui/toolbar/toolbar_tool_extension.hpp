#pragma once
#include <atomic>
#include <imgui.h>
#include <core/types.hpp>

namespace vt::ui
{
	struct toolbar_tool_extension
	{
	public:
		toolbar_tool_extension() = default;
		toolbar_tool_extension(const toolbar_tool_extension& other) : is_busy_{} {}
		toolbar_tool_extension(toolbar_tool_extension&& other) noexcept : is_busy_{} {}

	private:
		std::atomic<bool> is_busy_{};

	public:
		virtual uint32_t property_column_count() const
		{
			return 0;
		}

		void set_busy(bool value)
		{
			is_busy_.store(value, std::memory_order_release);
		}

		bool is_busy() const
		{
			return is_busy_.load(std::memory_order_acquire);
		}

		toolbar_tool_extension& operator=(const toolbar_tool_extension& other)
		{
			is_busy_ = false;
			return *this;
		}

		toolbar_tool_extension& operator=(toolbar_tool_extension&& other) noexcept
		{
			is_busy_ = false;
			return *this;
		}

		///@brief Called when the tool is activated
		virtual void on_activate() {}
		///@brief Called when the tool's context has changed
		virtual void on_switch_context() {}
		///@brief Called when the tool is deactivated
		virtual void on_deativate() {}

		///@brief Called when the "Done" button is pressed in the tool's popup.
		virtual void on_done() {}
		///@brief Called when any button other than "Done" is pressed in the tool's popup. The button ID is passed as an argument.
		virtual void on_button_click(int id) {}

		virtual void render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size) {}

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
