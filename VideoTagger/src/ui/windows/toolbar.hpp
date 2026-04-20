#pragma once
#include <memory>
#include <vector>
#include <string>
#include <ui/window.hpp>
#include <ui/toolbar_tool.hpp>

namespace vt::ui::windows
{
	struct toolbar : public window
	{
	public:
		toolbar();

	private:
		std::vector<std::unique_ptr<toolbar_tool>> tools_;
		std::string active_tool_;

	public:
		void add_tool(const toolbar_tool& tool);
		void remove_tool(const std::string& tool_id);
		void clear_tools();

		virtual void pre_style() override;
		virtual void post_style() override;

		virtual void on_render() override;
	private:
		void add_default_tools();
		void register_listeners();
	};
}
