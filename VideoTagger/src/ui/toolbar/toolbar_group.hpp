#pragma once
#include <vector>
#include <string>
#include <memory>

#include <events/event_source.hpp>
#include <ui/toolbar/toolbar_tool.hpp>
#include <ui/toolbar/toolbar_group_entry.hpp>
#include <ui/toolbar/toolbar_tool_specification.hpp>

namespace vt::ui
{
	struct toolbar_group
	{
	public:
		using tool_container = std::unordered_map<std::string, toolbar_group_entry>;
		using iterator = tool_container::iterator;
		using const_iterator = tool_container::const_iterator;

		toolbar_group() = default;

	private:
		///@brief Maps tool IDs to their corresponding group entries.
		tool_container entries_;

	public:
		void add_tool(event_source source, const toolbar_tool_specification& tool_spec, const toolbar_tool& tool = {}, size_t sort_index = 0);
		void add_tool(event_source source, const toolbar_tool_specification& tool_spec, std::unique_ptr<toolbar_tool>&& tool, size_t sort_index = 0);
		bool remove_tool(event_source source, const std::string& tool_id);
		void remove_non_persistent(event_source source);

		void clear(event_source source);

		iterator find(const std::string& tool_id);
		const_iterator find(const std::string& tool_id) const;
		bool contains(const std::string& tool_id) const;

		size_t size() const;
		bool empty() const;

		iterator begin();
		const_iterator begin() const;
		iterator end();
		const_iterator end() const;

		std::vector<std::pair<std::string, toolbar_group_entry*>> entries_sorted();
		std::vector<std::pair<std::string, const toolbar_group_entry*>> entries_sorted() const;

		toolbar_group_entry& operator[](const std::string& tool_id);
		const toolbar_group_entry& operator[](const std::string& tool_id) const;
	};
}
