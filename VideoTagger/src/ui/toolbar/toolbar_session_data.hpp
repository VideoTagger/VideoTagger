#pragma once
#include <memory>
#include <string>
#include <map>

#include <core/types.hpp>
#include <impl/resettable.hpp>
#include <events/event_source.hpp>

namespace vt::ui
{
	struct toolbar_group;

	struct toolbar_session_data : public vt::impl::resettable
	{
	public:
		toolbar_session_data();

	private:
		std::map<std::string, ui::toolbar_group> groups_;
		std::string active_tool_;
		event_source source_;

	public:
		void remove_non_persistent(event_source source);
		void clear_tools(event_source source);
		void request_register_tools(event_source source);
		bool is_tool_active(const std::string& tool_id) const;
		void reset_active_tool(event_source source);

		ui::toolbar_group& group(const std::string& group_id);
		const ui::toolbar_group& group(const std::string& group_id) const;
		std::map<std::string, ui::toolbar_group>& groups();
		const std::map<std::string, ui::toolbar_group>& groups() const;
		const std::string& active_tool() const;

		virtual void reset() override;

	private:
		void add_default_tools(event_source source);
		void register_listeners(event_source source);
	};
}
