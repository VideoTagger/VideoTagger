#pragma once
#include <memory>
#include <ui/window.hpp>
#include <ui/toolbar/toolbar_session_data.hpp>

namespace vt::ui::windows
{
	struct tool_properties : public window
	{
	public:
		tool_properties();

	private:
		bool reset_pos_;

	public:
		virtual void pre_style() override;
		virtual void post_style() override;

		virtual void on_render() override;
	private:
		toolbar_session_data& data();
	};
}
