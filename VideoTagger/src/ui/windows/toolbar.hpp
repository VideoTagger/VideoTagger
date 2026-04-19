#pragma once
#include <ui/window.hpp>

namespace vt::ui::windows
{
	struct toolbar_tool
	{
		std::string icon;
		std::string tooltip;
		std::function<void()> on_click;
	};

	struct toolbar : public window
	{
	public:
		toolbar();

	private:
		std::vector<toolbar_tool> tools_;
		size_t active_tool_;

	public:
		void add_tool(const toolbar_tool& tool);

		virtual void pre_style() override;
		virtual void post_style() override;

		virtual void on_render() override;
	};
}
