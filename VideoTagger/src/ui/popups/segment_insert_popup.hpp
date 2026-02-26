#pragma once
#include <vector>
#include <string>
#include <memory>
#include <ui/popup.hpp>
#include <ui/widgets/combo.hpp>
#include <tags/tag_timeline.hpp>
#include <events/timeline/segment_insert_request_event.hpp>

namespace vt::ui
{
	///@brief Popup displayed when creating a new segment
	class segment_insert_popup : public modal_popup
	{
	public:
		segment_insert_popup(const segment_insert_request_event& event_data, const std::vector<std::string>& tags,
		timestamp min_timestamp, timestamp max_timestamp, std::optional<bool*> open = std::nullopt);

	private:
		segment_insert_request_event insert_request_event_data_;
		bool accepted_ = false;
		std::vector<std::string> tag_names_;
		timestamp start_;
		timestamp end_;
		timestamp min_timestamp_;
		timestamp max_timestamp_;
		ui::combo<std::string> tag_combo_;

	public:
		virtual void on_render() override;
		virtual void on_close() override;
	};
}
