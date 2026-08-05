#pragma once
#include <ostream>
#include <string>
#include <ui/popup.hpp>
#include <ui/widgets/combo.hpp>
#include <ui/toolbar/toolbar_group_entry.hpp>

namespace vt::ui
{
	struct toolbar_popup_entry
	{
		std::string display_name;
		toolbar_tool* tool{};

		friend std::ostream& operator<<(std::ostream& os, const toolbar_popup_entry& entry);
	};

	struct toolbar_tool_popup : public popup
	{
	public:
		toolbar_tool_popup();

	private:
		combo<toolbar_popup_entry> popup_entries_;
		toolbar_group_entry* active_entry_;
		ImVec2 pos_;

	public:
		void reset_entries();

		void set_active_entry(toolbar_group_entry* entry);
		void set_position(ImVec2 pos);

		ImVec2 position() const;

		virtual void pre_style() override;
		virtual void post_style() override;

		virtual void on_display() override;
		virtual void on_render() override;
	private:
		void render_tool_combo();
		void render_body();
	};
}
