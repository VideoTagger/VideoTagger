#pragma once
#include <string>
#include <vector>
#include <cstdint>

#include <ui/popup.hpp>

#include <tags/tag_storage.hpp>
#include <events/event_source.hpp>
#include <attributes/impl/shape_attribute_instance.hpp>

#include <ui/widgets/combo.hpp>

namespace vt::ui
{
	enum class track_which_regions : uint8_t
	{
		all_visible,
		selected,
		custom
	};

	struct track_region_popup : public modal_popup
	{
	public:
		track_region_popup(const std::vector<region_info>& initial_regions, timestamp current_ts);
		track_region_popup(const std::type_info& shape_type_info, timestamp current_ts, track_which_regions which_regions);

	private:
		const std::type_info* shape_type_info_{};
		std::vector<region_info> tracked_regions_;
		combo<std::string> trackers_combo_;
		combo<std::string> which_regions_combo_;
		timestamp current_ts_;
		timestamp max_ts_;
		timestamp target_ts_;
		bool replace_keyframes_{ false };
		event_source event_source_;
		track_which_regions which_regions_{};

	public:
		virtual void on_display() override;
		virtual void on_render() override;
		virtual void on_close() override;

	private:
		void update_max_timestamp();
		void update_region_list(track_which_regions which_regions);
	};
}
