#pragma once
#include <vector>
#include <memory>
#include <ui/toolbar/toolbar_tool.hpp>
#include <ui/toolbar/toolbar_session_data.hpp>
#include <ui/toolbar/toolbar_tool_specification.hpp>

namespace vt::ui
{
	struct toolbar_group;

	struct toolbar_group_entry
	{
	public:
		using tool_container = std::vector<std::unique_ptr<toolbar_tool>>;
		using iterator = tool_container::iterator;
		using const_iterator = tool_container::const_iterator;

		toolbar_group_entry(toolbar_group& group, const toolbar_tool_specification& spec = {});

	private:
		tool_container tools_;
		toolbar_tool_specification spec_;
		toolbar_group* group_;
		toolbar_tool* active_tool_;
		size_t sort_index_;

	public:
		void add_tool(event_source source, const toolbar_tool& tool);
		void add_tool(event_source source, std::unique_ptr<toolbar_tool>&& tool);

		void clear(event_source source);

		void set_sort_index(size_t index);
		void set_active_tool(toolbar_tool& tool);

		toolbar_tool* active_tool();
		const toolbar_tool* active_tool() const;

		iterator begin();
		const_iterator begin() const;
		iterator end();
		const_iterator end() const;

		std::unique_ptr<toolbar_tool>& front();
		const std::unique_ptr<toolbar_tool>& front() const;
		std::unique_ptr<toolbar_tool>& back();
		const std::unique_ptr<toolbar_tool>& back() const;

		const tool_container& tools() const;
		toolbar_group& group();
		const toolbar_group& group() const;
		const toolbar_tool_specification& specification() const;

		const std::string& tool_id() const;
		size_t tool_count() const;
		bool empty() const;
		bool has_id(const std::string& tool_id) const;
		bool has_any_tool_body() const;

		void on_activate();
		void on_deactivate();
		void on_done();

		bool operator<(const toolbar_group_entry& other) const;
	};
}
