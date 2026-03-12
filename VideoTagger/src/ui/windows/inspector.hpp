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
		timestamp min_timestamp_;
		timestamp max_timestamp_;
		timestamp current_offset_;
		segment_part grab_part_{};
		bool link_segment_parts_;

	public:
		[[nodiscard]] virtual nlohmann::ordered_json serialize() const override;
		virtual void deserialize(const nlohmann::ordered_json& json) override;

		virtual void on_render() override;

	private:
		void register_listeners();

		std::pair<std::string, segment_id> first_selected_segment() const;
	};
}
