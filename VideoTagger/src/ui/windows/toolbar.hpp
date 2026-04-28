#pragma once
#include <ui/window.hpp>
#include <ui/toolbar_tool.hpp>

namespace vt::ui::windows
{
	struct toolbar : public window
	{
	public:
		toolbar();

	private:
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
