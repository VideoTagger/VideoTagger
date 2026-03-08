#pragma once
#include <ui/window.hpp>
#include <tags/tag_timeline.hpp>

namespace vt::ui::windows
{
	class inspector : public window
	{
	public:
		inspector();

	private:
		segment_id_map selected_segments_;
		segment_id_map dragged_segments_;
		timestamp min_timestamp_;
		timestamp max_timestamp_;
		timestamp current_offset_;
		segment_part grab_part_{};
		event_source drag_source_;
		bool link_segment_parts_;

	public:
		[[nodiscard]] virtual nlohmann::ordered_json serialize() const override;
		virtual void deserialize(const nlohmann::ordered_json& json) override;

		virtual void on_render() override;

	private:
		void register_listeners();

		bool is_segment_selected(const std::string& tag, segment_id segment) const;
		bool is_any_segment_selected() const;
		bool is_segment_dragged(const std::string& tag, segment_id segment) const;
		bool is_dragging_any_segment() const;

		bool more_than_one_segment_selected() const;

		std::pair<std::string, segment_id> first_selected_segment() const;
	};
}
