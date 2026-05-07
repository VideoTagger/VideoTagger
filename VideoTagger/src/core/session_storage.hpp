#pragma once
#include <optional>
#include <vector>
#include <string>

#include <utils/timestamp.hpp>
#include <tags/tag_timeline.hpp>
#include <video/video_pool.hpp>
#include <impl/resettable.hpp>
#include <events/event_source.hpp>
#include <utils/vec.hpp>
#include <tasks/session_task_manager.hpp>
#include <attributes/impl/attribute_instance.hpp>
#include <ui/toolbar_tool.hpp>
#include <core/types.hpp>

namespace vt
{
	struct insert_segment_mark_data
	{
		std::optional<std::string> tag;
		timestamp start;
		uint64_t mark_id{};
	};

	enum class segment_drag_stage
	{
		not_dragging,
		dragging,
		waiting_for_approval,
	};

	struct segment_drag_data
	{
		segment_part grab_part{ segment_part::none };
		segment_drag_stage stage{ segment_drag_stage::not_dragging };
		timestamp current_offset{};
		event_source begin_drag_source{};
	};

	struct selected_region_data
	{
		std::string tag_name;
		segment_id segment;
		impl::attribute_instance* attribute_instance{};
		region_id_t region_id{};

		constexpr bool operator==(const selected_region_data& other) const
		{
			return attribute_instance == other.attribute_instance and region_id == other.region_id;
		}

		constexpr bool operator!=(const selected_region_data& other) const
		{
			return !(*this == other);
		}
	};

	struct gizmo_data
	{
		video_id_t video_id{};
		std::vector<utils::vec2<uint32_t>*> targets;
	};

	///@brief Storage for temporary data related to the current session
	class session_storage : public impl::resettable
	{
	public:
		session_storage();

	private:
		segment_id_map selected_segments_;
		segment_id_map dragged_segments_;
		segment_drag_data segment_drag_data_;
		video_group_id_t current_video_group_id_{ invalid_video_group_id };
		std::vector<insert_segment_mark_data> insert_segment_marks_;

		std::optional<selected_region_data> selected_region_;

		gizmo_data gizmo_data_;

		std::vector<selected_region_data> hovered_regions_;

		event_source event_source_{ "session" };

		void register_timeline_listeners();
		void register_gizmo_listeners();
		void register_attribute_listeners();
	
	public:
		session_task_manager tasks;
		ui::toolbar_session_data toolbar;
		
		const segment_id_map& selected_segments() const;
		const segment_id_map& dragged_segments() const;
		const segment_drag_data& segment_drag_data() const;
		video_group_id_t current_video_group_id() const;
		bool is_any_video_group_active() const;
		const std::vector<insert_segment_mark_data>& insert_segment_marks() const;

		const std::optional<selected_region_data>& selected_region() const;
		bool is_region_selected(impl::attribute_instance* attribute_instance, region_id_t region_id) const;
		bool is_any_region_selected() const;

		const std::vector<selected_region_data>& hovered_regions() const;
		bool is_region_hovered(impl::attribute_instance* attribute_instance, region_id_t region_id) const;
		bool is_any_region_hovered() const;

		bool is_segment_selected(const std::string& tag, segment_id id) const;
		bool is_one_segment_selected() const;
		bool is_any_segment_selected() const;
		bool is_more_than_one_segment_selected() const;
		std::optional<std::pair<std::string, segment_id>> any_selected_segment() const;
		std::optional<segment_id> any_selected_segment(const std::string& tag) const;

		bool is_segment_dragged(const std::string& tag, segment_id id) const;
		bool is_dragging_any_segment() const;

		bool gizmo_contains_target(const utils::vec2<uint32_t>* target) const;
		video_id_t gizmo_video_id() const;
		const std::vector<utils::vec2<uint32_t>*>& gizmo_targets() const;
		bool has_gizmo_targets() const;
		utils::vec2<uint32_t> mean_gizmo_target() const;

		std::vector<insert_segment_mark_data>::iterator find_insert_segment_mark_by_tag(const std::string& tag);
		std::vector<insert_segment_mark_data>::iterator find_insert_segment_mark_by_tag(const std::optional<std::string>& tag);
		std::vector<insert_segment_mark_data>::iterator find_insert_segment_mark_by_id(uint64_t id);

		virtual void reset() override;
	};
}
