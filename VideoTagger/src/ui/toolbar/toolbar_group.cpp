#include "toolbar_group.hpp"
#include <core/app_context.hpp>
#include <events/toolbar/toolbar_unregister_tool_event.hpp>

namespace vt::ui
{
	void toolbar_group::add_tool(event_source source, const toolbar_tool_specification& tool_spec, const toolbar_tool& tool, size_t sort_index)
	{
		auto it = find(tool_spec.id);
		if (it == entries_.end())
		{
			it = entries_.emplace(tool_spec.id, toolbar_group_entry{ *this, tool_spec }).first;
		}
		auto& entry = it->second;
		entry.set_sort_index(sort_index);
		entry.add_tool(source, tool);
	}

	void toolbar_group::add_tool(event_source source, const toolbar_tool_specification& tool_spec, std::unique_ptr<toolbar_tool>&& tool, size_t sort_index)
	{
		auto ptr = tool.get();
		auto it = find(tool_spec.id);
		if (it == entries_.end())
		{
			it = entries_.emplace(tool_spec.id, toolbar_group_entry{ *this, tool_spec }).first;
		}
		auto& entry = it->second;
		entry.set_sort_index(sort_index);
		entry.add_tool(source, std::move(tool));
	}

	bool toolbar_group::remove_tool(event_source source, const std::string& tool_id)
	{
		auto it = find(tool_id);
		if (it != entries_.end())
		{
			auto& entry = it->second;
			entry.clear(source);
			entries_.erase(it);
			return true;
		}
		return false;
	}

	void toolbar_group::remove_non_persistent(event_source source)
	{
		for (auto it = entries_.begin(); it != entries_.end();)
		{
			auto& entry = it->second;
			const auto& spec = entry.specification();
			if (!spec.is_persistent)
			{
				for (const auto& tool : entry.tools())
				{
					ctx_.dispatch_event<toolbar_unregister_tool_event>(source, *this, entry, *tool);
				}
				it = entries_.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	void toolbar_group::clear(event_source source)
	{
		for (const auto& [id, entry] : entries_)
		{
			for (const auto& tool : entry.tools())
			{
				ctx_.dispatch_event<toolbar_unregister_tool_event>(source, *this, entry, *tool);
			}
		}
		entries_.clear();
	}

	toolbar_group::iterator toolbar_group::find(const std::string& tool_id)
	{
		return entries_.find(tool_id);
	}

	toolbar_group::const_iterator toolbar_group::find(const std::string& tool_id) const
	{
		return entries_.find(tool_id);
	}

	bool toolbar_group::contains(const std::string& tool_id) const
	{
		return entries_.find(tool_id) != entries_.end();
	}

	size_t toolbar_group::size() const
	{
		return entries_.size();
	}

	bool toolbar_group::empty() const
	{
		return entries_.empty();
	}

	toolbar_group::iterator toolbar_group::begin()
	{
		return entries_.begin();
	}

	toolbar_group::const_iterator toolbar_group::begin() const
	{
		return entries_.begin();
	}

	toolbar_group::iterator toolbar_group::end()
	{
		return entries_.end();
	}

	toolbar_group::const_iterator toolbar_group::end() const
	{
		return entries_.end();
	}

	std::vector<std::pair<std::string, toolbar_group_entry*>> toolbar_group::entries_sorted()
	{
		std::vector<std::pair<std::string, toolbar_group_entry*>> sorted_entries;
		for (auto& [id, entry] : entries_)
		{
			sorted_entries.emplace_back(id, &entry);
		}
		std::sort(sorted_entries.begin(), sorted_entries.end(), [](const std::pair<std::string, toolbar_group_entry*>& a, const std::pair<std::string, toolbar_group_entry*>& b)
		{
			return *a.second < *b.second;
		});
		return sorted_entries;
	}

	std::vector<std::pair<std::string, const toolbar_group_entry*>> toolbar_group::entries_sorted() const
	{
		std::vector<std::pair<std::string, const toolbar_group_entry*>> sorted_entries;
		for (const auto& [id, entry] : entries_)
		{
			sorted_entries.emplace_back(id, &entry);
		}
		std::sort(sorted_entries.begin(), sorted_entries.end(), [](const std::pair<std::string, const toolbar_group_entry*>& a, const std::pair<std::string, const toolbar_group_entry*>& b)
		{
			return *a.second < *b.second;
		});
		return sorted_entries;
	}

	toolbar_group_entry& toolbar_group::operator[](const std::string& tool_id)
	{
		auto it = entries_.find(tool_id);
		if (it == entries_.end())
		{
			it = entries_.emplace(tool_id, toolbar_group_entry{ *this }).first;
		}
		return it->second;
	}

	const toolbar_group_entry& toolbar_group::operator[](const std::string& tool_id) const
	{
		return entries_.at(tool_id);
	}
}
