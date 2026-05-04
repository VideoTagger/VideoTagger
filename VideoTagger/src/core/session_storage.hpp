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

		impl::attribute_instance* selected_attribute_instance_{};

		std::vector<utils::vec2<uint32_t>*> gizmo_targets_;

		event_source event_source_{ "session" };

	public:
		session_task_manager tasks;
		ui::toolbar_session_data toolbar;
		
		void register_timeline_listeners();
		void register_gizmo_listeners();
		
		const segment_id_map& selected_segments() const;
		const segment_id_map& dragged_segments() const;
		const segment_drag_data& segment_drag_data() const;
		video_group_id_t current_video_group_id() const;
		const std::vector<insert_segment_mark_data>& insert_segment_marks() const;

		const impl::attribute_instance* selected_attribute_instance() const;

		bool is_segment_selected(const std::string& tag, segment_id id) const;
		bool is_one_segment_selected() const;
		bool is_any_segment_selected() const;
		bool is_more_than_one_segment_selected() const;
		std::optional<std::pair<std::string, segment_id>> any_selected_segment() const;
		std::optional<segment_id> any_selected_segment(const std::string& tag) const;

		bool is_segment_dragged(const std::string& tag, segment_id id) const;
		bool is_dragging_any_segment() const;

		bool gizmo_contains_target(const utils::vec2<uint32_t>* target) const;
		std::vector<utils::vec2<uint32_t>*> gizmo_targets();
		const std::vector<utils::vec2<uint32_t>*>& gizmo_targets() const;
		bool has_gizmo_targets() const;
		utils::vec2<uint32_t> mean_gizmo_target() const;

		std::vector<insert_segment_mark_data>::iterator find_insert_segment_mark_by_tag(const std::string& tag);
		std::vector<insert_segment_mark_data>::iterator find_insert_segment_mark_by_tag(const std::optional<std::string>& tag);
		std::vector<insert_segment_mark_data>::iterator find_insert_segment_mark_by_id(uint64_t id);

		virtual void reset() override;
	};
}
