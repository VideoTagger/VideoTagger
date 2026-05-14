#include "toolbar_group_entry.hpp"
#include <ui/toolbar/toolbar_group.hpp>
#include <events/toolbar/toolbar_unregister_tool_event.hpp>
#include <core/app_context.hpp>
#include <events/toolbar/toolbar_register_tool_event.hpp>

namespace vt::ui
{
	toolbar_group_entry::toolbar_group_entry(toolbar_group& group, const toolbar_tool_specification& spec) : group_{ &group }, spec_{ spec }, active_tool_{}, sort_index_{} {}

	void toolbar_group_entry::add_tool(event_source source, const toolbar_tool& tool)
	{
		auto tool_ptr = std::make_unique<toolbar_tool>(tool);
		auto ptr = tool_ptr.get();

		if (active_tool_ == nullptr)
		{
			set_active_tool(*ptr);
		}
		tools_.push_back(std::move(tool_ptr));
		ctx_.dispatch_event<toolbar_register_tool_event>(source, *group_, *this, *ptr);
	}

	void toolbar_group_entry::add_tool(event_source source, std::unique_ptr<toolbar_tool>&& tool)
	{
		auto ptr = tool.get();
		tools_.push_back(std::move(tool));
		ctx_.dispatch_event<toolbar_register_tool_event>(source, *group_, *this, *ptr);
	}

	void toolbar_group_entry::clear(event_source source)
	{
		for (const auto& tool : tools_)
		{
			ctx_.dispatch_event<toolbar_unregister_tool_event>(source, *group_, *this, *tool);
		}
		tools_.clear();
	}

	void toolbar_group_entry::set_sort_index(size_t index)
	{
		sort_index_ = index;
	}

	void toolbar_group_entry::set_active_tool(toolbar_tool& tool)
	{
		active_tool_ = &tool;
	}

	toolbar_tool* toolbar_group_entry::active_tool()
	{
		return active_tool_;
	}

	const toolbar_tool* toolbar_group_entry::active_tool() const
	{
		return active_tool_;
	}

	toolbar_group_entry::iterator toolbar_group_entry::begin()
	{
		return tools_.begin();
	}

	toolbar_group_entry::const_iterator toolbar_group_entry::begin() const
	{
		return tools_.begin();
	}

	toolbar_group_entry::iterator toolbar_group_entry::end()
	{
		return tools_.end();
	}

	toolbar_group_entry::const_iterator toolbar_group_entry::end() const
	{
		return tools_.end();
	}

	std::unique_ptr<toolbar_tool>& toolbar_group_entry::front()
	{
		return tools_.front();
	}

	const std::unique_ptr<toolbar_tool>& toolbar_group_entry::front() const
	{
		return tools_.front();
	}

	std::unique_ptr<toolbar_tool>& toolbar_group_entry::back()
	{
		return tools_.back();
	}

	const std::unique_ptr<toolbar_tool>& toolbar_group_entry::back() const
	{
		return tools_.back();
	}

	const toolbar_group_entry::tool_container& toolbar_group_entry::tools() const
	{
		return tools_;
	}

	toolbar_group& toolbar_group_entry::group()
	{
		return *group_;
	}

	const toolbar_group& toolbar_group_entry::group() const
	{
		return *group_;
	}

	const toolbar_tool_specification& toolbar_group_entry::specification() const
	{
		return spec_;
	}

    const std::string& toolbar_group_entry::tool_id() const
    {
		return spec_.id;
    }

    size_t toolbar_group_entry::tool_count() const
	{
		return tools_.size();
	}

	bool toolbar_group_entry::empty() const
	{
		return tools_.empty();
	}

	bool toolbar_group_entry::has_id(const std::string& id) const
	{
		return spec_.id == id;
	}

	bool toolbar_group_entry::has_any_tool_body() const
	{
		for (const auto& tool : tools_)
		{
			if (tool->has_body())
			{
				return true;
			}
		}
		return false;
	}

	void toolbar_group_entry::on_activate()
	{
		if (active_tool_ != nullptr)
		{
			active_tool_->on_activate();
		}
	}

	void toolbar_group_entry::on_deactivate()
	{
		if (active_tool_ != nullptr)
		{
			active_tool_->on_deativate();
		}
	}

	void toolbar_group_entry::on_done()
	{
		if (active_tool_ != nullptr)
		{
			active_tool_->on_done();
		}
	}

	void toolbar_group_entry::on_button_click(int id)
	{
		if (active_tool_ != nullptr)
		{
			active_tool_->on_button_click(id);
		}
	}

	bool toolbar_group_entry::operator<(const toolbar_group_entry& other) const
	{
		if (spec_.is_persistent and !other.spec_.is_persistent) return true;
		if (!spec_.is_persistent and other.spec_.is_persistent) return false;
		if (sort_index_ == other.sort_index_)
		{
			return spec_.id < other.spec_.id;
		}
		return sort_index_ < other.sort_index_;
	}
}
