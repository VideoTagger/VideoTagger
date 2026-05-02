#pragma once
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

#include <impl/resettable.hpp>
#include <events/event_source.hpp>

namespace vt::ui
{
	struct toolbar_tool
	{
		std::string id;
		std::string icon;
		std::string tooltip;
		///@brief If ture, the tool won't be deleted during the tool re-registration phase.
		bool is_persistent = false;
	};

	struct toolbar_session_data : public vt::impl::resettable
	{
	public:
		toolbar_session_data();

	private:
		std::unordered_map<std::string, std::vector<std::unique_ptr<toolbar_tool>>> tools_;
		std::string active_tool_;
		event_source source_;

	public:
		void add_tool(event_source source, const toolbar_tool& tool);
		void remove_tool(event_source source, const std::string& tool_id);
		void remove_non_persistent(event_source source);
		void clear_tools(event_source source);
		void request_register_tools(event_source source);
		bool is_tool_active(const std::string& tool_id) const;

		const std::unordered_map<std::string, std::vector<std::unique_ptr<toolbar_tool>>>& tools() const;
		const std::string& active_tool() const;

		virtual void reset() override;

	private:
		void add_default_tools(event_source source);
		void register_listeners(event_source source);
	};
}
