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

		virtual std::string display_name() const
		{
			return "unknown-tool";
		}

		virtual void render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size) {}
		virtual void render_popup_body() {}
	};
}
