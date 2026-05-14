#include "toolbar_session_data.hpp"

#include <core/app_context.hpp>
#include <ui/toolbar/toolbar_group.hpp>
#include <ui/toolbar/toolbar_group_entry.hpp>
#include <events/toolbar/toolbar_register_tool_event.hpp>
#include <events/toolbar/toolbar_unregister_tool_event.hpp>
#include <events/toolbar/toolbar_register_request_event.hpp>
#include <events/toolbar/toolbar_tool_changed_event.hpp>
#include <events/toolbar/toolbar_tool_change_request.hpp>

#include <events/attributes/attribute_added_event.hpp>
#include <events/attributes/attribute_deleted_event.hpp>

namespace vt::ui
{
	toolbar_session_data::toolbar_session_data() : source_{ "toolbar-session-data" }, active_entry_{}
	{
		register_listeners(source_);
		add_default_tools(source_);
	}

	void toolbar_session_data::remove_non_persistent(event_source source)
	{
		for (auto& [id, group] : groups_)
		{
			group.remove_non_persistent(source);
		}
	}

	void toolbar_session_data::clear_tools(event_source source)
	{
		for (auto& [tool_id, group] : groups_)
		{
			group.clear(source);
		}
	}

	void toolbar_session_data::request_register_tools(event_source source)
	{
		ctx_.dispatch_event<toolbar_register_request_event>(source);
	}

	bool toolbar_session_data::is_tool_active(const std::string& tool_id) const
	{
		return active_entry_ != nullptr and active_entry_->has_id(tool_id);
	}

	void toolbar_session_data::reset_active_tool(event_source source)
	{
		if (active_entry_ != nullptr)
		{
			for (auto& [id, group] : groups_)
			{
				for (auto& [id, entry] : group)
				{
					const auto& spec = entry.specification();
					if (spec.is_persistent)
					{
						for (auto& tool : entry)
						{
							ctx_.dispatch_event<toolbar_tool_change_request_event>(source, group, entry, *tool);
							return;
						}
					}
				}
			}
			//ctx_.dispatch_event<toolbar_tool_changed_event>(source, toolbar_tool{});
		}
	}

	ui::toolbar_group& toolbar_session_data::group(const std::string& group_id)
	{
		return groups_[group_id];
	}

	const ui::toolbar_group& toolbar_session_data::group(const std::string& group_id) const
	{
		return groups_.at(group_id);
	}

	std::map<std::string, ui::toolbar_group>& toolbar_session_data::groups()
	{
		return groups_;
	}

	const std::map<std::string, ui::toolbar_group>& toolbar_session_data::groups() const
	{
		return groups_;
	}

	toolbar_group_entry* toolbar_session_data::active_entry()
	{
		return active_entry_;
	}

	const toolbar_group_entry* toolbar_session_data::active_entry() const
	{
		return active_entry_;
	}

	void toolbar_session_data::reset()
	{
		remove_non_persistent(source_);
	}

	void toolbar_session_data::add_default_tools(event_source source)
	{
		auto& default_group = group("default");

		size_t idx{};
		default_group.add_tool(source, toolbar_tool_specification{ "select", icons::tool_arrow, "Select", true }, toolbar_tool{}, idx++);
		default_group.add_tool(source, toolbar_tool_specification{ "move", icons::tool_move, "Move", true }, toolbar_tool{}, idx++);
		default_group.add_tool(source, toolbar_tool_specification{ "magnifier", icons::search, "Zoom In/Out", true }, toolbar_tool{}, idx++);
	}

	void toolbar_session_data::register_listeners(event_source source)
	{
		//TODO: Unregister listeners in destructor
		ctx_.add_event_listener<toolbar_register_tool_event>([this, source](const toolbar_register_tool_event& event)
		{
			if (active_entry_ == nullptr)
			{
				ctx_.dispatch_event<toolbar_tool_change_request_event>(source, event.group(), event.group_entry(), event.tool());
			}
		});

		ctx_.add_event_listener<toolbar_unregister_tool_event>([this, source](const toolbar_unregister_tool_event& event)
		{
			const auto& entry = event.group_entry();
			const auto& spec = entry.specification();
			if (active_entry_ != nullptr and active_entry_->has_id(spec.id))
			{
				reset_active_tool(source);
			}
		});

		ctx_.add_event_listener<toolbar_tool_change_request_event>([this](const toolbar_tool_change_request_event& event)
		{
			auto& group = event.group();
			auto& entry = event.group_entry();
			const auto& spec = entry.specification();
			auto& tool = event.tool();
			const auto& new_id = spec.id;

			bool is_null = active_entry_ == nullptr;
			if (!is_null and active_entry_->has_id(new_id)) return;

			if (new_id.empty())
			{
				if (!is_null)
				{
					active_entry_->on_deactivate();
				}
				active_entry_ = nullptr;
				debug::log("Toolbar: Active tool cleared");
			}
			else
			{
				debug::log("Toolbar: Active tool changed to '{}'", new_id);
				if (!is_null)
				{
					active_entry_->on_deactivate();
				}
				active_entry_ = &entry;
				active_entry_->set_active_tool(tool);
				if (!is_null)
				{
					active_entry_->on_activate();
				}
			}
			ctx_.dispatch_event<toolbar_tool_changed_event>(event.source(), group, entry, tool);
		});

		//ctx_.add_event_listener<toolbar_tool_changed_event>([this](const toolbar_tool_changed_event& event)
		//{
		//	
		//});

		ctx_.add_event_listener<attribute_added_event>([this, source](const attribute_added_event& event)
		{
			remove_non_persistent(source);
			request_register_tools(source);
		});

		ctx_.add_event_listener<attribute_deleted_event>([this, source](const attribute_deleted_event& event)
		{
			remove_non_persistent(source);
			request_register_tools(source);
		});
	}
}
