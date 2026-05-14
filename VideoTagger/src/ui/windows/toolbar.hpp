#pragma once
#include <memory>
#include <ui/window.hpp>
#include <ui/toolbar/toolbar_session_data.hpp>
#include <ui/popups/toolbar_tool_popup.hpp>

namespace vt::ui::windows
{
	struct toolbar : public window
	{
	public:
		toolbar();

	private:
		std::unique_ptr<ui::toolbar_tool_popup> tool_popup_;
		bool reset_pos_;

	public:
		virtual void pre_style() override;
		virtual void post_style() override;

		virtual void on_render() override;
	private:
		toolbar_session_data& data();
		float calc_win_height();
	};
}
