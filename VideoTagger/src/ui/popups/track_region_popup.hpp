#pragma once
#include <string>
#include <vector>

#include <ui/popup.hpp>

#include <tags/tag_storage.hpp>
#include <events/event_source.hpp>
#include <attributes/impl/shape_attribute_instance.hpp>

#include <ui/widgets/combo.hpp>

namespace vt::ui
{
	struct track_region_popup : public modal_popup
	{
	public:
		track_region_popup(const std::string& active_tag, segment_id active_segment, video_id_t video_id, vt::impl::shape_attribute_instance& attr_instance,
			timestamp current_ts, region_id_t region_id, const std::vector<std::string>& trackers);

	private:
		std::string active_tag_;
		segment_id active_segment_{};
		video_id_t video_id_;
		vt::impl::shape_attribute_instance* attr_instance_{};
		region_id_t region_id_;
		combo<std::string> trackers_combo_;
		timestamp current_ts_;
		timestamp max_ts_;
		timestamp target_ts_;
		bool replace_keyframes_{ false };
		event_source event_source_;

	public:
		virtual void on_display() override;
		virtual void on_render() override;
		virtual void on_close() override;
	};
}
