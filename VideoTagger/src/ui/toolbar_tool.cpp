#include "toolbar_tool.hpp"

#include <core/app_context.hpp>
#include <events/toolbar/toolbar_register_tool_event.hpp>
#include <events/toolbar/toolbar_unregister_tool_event.hpp>
#include <events/toolbar/toolbar_register_request_event.hpp>
#include <events/toolbar/toolbar_tool_changed_event.hpp>

namespace vt::ui
{
	toolbar_session_data::toolbar_session_data() : source_{ "toolbar-session-data" }
	{
		register_listeners(source_);
		add_default_tools(source_);
	}

	void toolbar_session_data::add_tool(event_source source, const toolbar_tool& tool)
	{
		auto tool_ptr = std::make_unique<toolbar_tool>(tool);
		auto ptr = tool_ptr.get();
		tools_.push_back(std::move(tool_ptr));

		ctx_.dispatch_event<toolbar_register_tool_event>(source, *ptr);
	}

	void toolbar_session_data::remove_tool(event_source source, const std::string& tool_id)
	{
		auto& tools = tools_;
		auto it = std::find_if(tools.begin(), tools.end(), [&tool_id](const std::unique_ptr<toolbar_tool>& tool)
		{
			return tool->id == tool_id;
		});

		if (it != tools.end())
		{
			ctx_.dispatch_event<toolbar_unregister_tool_event>(source, *it->get());
			tools.erase(it);
		}
	}

	void toolbar_session_data::remove_non_persistent(event_source source)
	{
		for (auto it = tools_.begin(); it != tools_.end();)
		{
			if (!(*it)->is_persistent)
			{
				ctx_.dispatch_event<toolbar_unregister_tool_event>(source, *it->get());
				it = tools_.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	void toolbar_session_data::clear_tools(event_source source)
	{
		for (auto& tool : tools_)
		{
			ctx_.dispatch_event<toolbar_unregister_tool_event>(source, *tool);
		}
		tools_.clear();
	}

	void toolbar_session_data::request_register_tools(event_source source)
	{
		ctx_.dispatch_event<toolbar_register_request_event>(source);
	}

	bool toolbar_session_data::is_tool_active(const std::string& tool_id) const
	{
		return active_tool_ == tool_id;
	}

	const std::vector<std::unique_ptr<toolbar_tool>>& toolbar_session_data::tools() const
	{
		return tools_;
	}

	const std::string& toolbar_session_data::active_tool() const
	{
		return active_tool_;
	}

	void toolbar_session_data::reset()
	{
		remove_non_persistent(source_);
	}

	void toolbar_session_data::add_default_tools(event_source source)
	{
		toolbar_tool arrow_tool
		{
			"select",
			icons::tool_arrow,
			"Select",
			true,
		};
		add_tool(source, arrow_tool);
	}

	void toolbar_session_data::register_listeners(event_source source)
	{
		ctx_.add_event_listener<toolbar_register_tool_event>([this, source](const toolbar_register_tool_event& event)
		{
			if (active_tool_.empty())
			{
				ctx_.dispatch_event<toolbar_tool_changed_event>(source, event.tool());
			}
		});

		ctx_.add_event_listener<toolbar_unregister_tool_event>([this, source](const toolbar_unregister_tool_event& event)
		{
			if (active_tool_ == event.tool().id)
			{
				ctx_.dispatch_event<toolbar_tool_changed_event>(source, toolbar_tool{});
			}
		});

		ctx_.add_event_listener<toolbar_tool_changed_event>([this](const toolbar_tool_changed_event& event)
		{
			auto new_id = event.tool().id;
			if (active_tool_ == new_id) return;

			if (new_id.empty())
			{
				active_tool_.clear();
				debug::log("Toolbar: Active tool cleared");
			}
			else
			{
				debug::log("Toolbar: Active tool changed to '{}'", new_id);
				active_tool_ = new_id;
			}
		});
	}
}
