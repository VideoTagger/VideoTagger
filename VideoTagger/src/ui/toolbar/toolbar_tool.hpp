#pragma once
#include <string>
#include <imgui.h>
#include <core/types.hpp>

namespace vt::ui
{
	struct toolbar_tool
	{
	public:
		toolbar_tool() = default;

	private:
		bool has_body_ = false;

	public:
		void set_has_body(bool value)
		{
			has_body_ = value;
		}

		bool has_body() const
		{
			return has_body_;
		}

		///@return The name to be displayed in the tool's popup as a combo item.
		virtual std::string display_name() const
		{
			return "unknown-tool";
		}

		virtual void on_activate() {}
		virtual void on_deativate() {}

		///@brief Called when the "Done" button is pressed in the tool's popup. Only relevant if has_body() returns true.
		virtual void on_done() {}

		virtual void render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size) {}
		///@brief Renders the body of the tool's popup. Only relevant if has_body() returns true.
		virtual void render_popup_body() {}
	};
}
