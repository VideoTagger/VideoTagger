#include "session_storage.hpp"

#include <core/app_context.hpp>

#include <events/timeline/segment_select_request_event.hpp>
#include <events/timeline/segment_selected_event.hpp>
#include <events/timeline/segment_deselect_request_event.hpp>
#include <events/timeline/segment_deselected_event.hpp>
#include <events/timeline/segment_insert_mark_start.hpp>
#include <events/timeline/segment_insert_mark_end.hpp>
#include <events/timeline/begin_segment_drag_event.hpp>
#include <events/timeline/update_segment_drag_event.hpp>
#include <events/timeline/end_segment_drag_event.hpp>
#include <events/timeline/segments_moved_event.hpp>
#include <events/timeline/segment_merged_event.hpp>
#include <events/timeline/segment_deleted_event.hpp>

#include <events/tags/tag_renamed_event.hpp>
#include <events/tags/tag_deleted_event.hpp>


#include <events/player/video_group_change_request_event.hpp>
#include <events/player/video_group_changed_event.hpp>


namespace vt
{
	session_storage::session_storage() :
		tasks{ ctx_.tasks }
	{
		ctx_.add_event_listener<segment_select_request_event>([this](const segment_select_request_event& event)
		{
			const auto& segment_strg = ctx_.get_current_segment_storage();
			if (&event.storage() != &segment_strg) return;

			auto [_, inserted] = selected_segments_[event.tag()].insert(event.id());
			if (!inserted)
			{
				return;
			}

			ctx_.dispatch_event<segment_selected_event>(event.source(), event.storage(), event.tag(), event.id());
		});

		ctx_.add_event_listener<segment_deselect_request_event>([this](const segment_deselect_request_event& event)
		{
			const auto& segment_strg = ctx_.get_current_segment_storage();
			if (&event.storage() != &segment_strg) return;

			if (!segment_id_map_erase(selected_segments_, event.tag(), event.id()))
			{
				return;
			}

			ctx_.dispatch_event<segment_selected_event>(event.source(), event.storage(), event.tag(), event.id());
		});

		ctx_.add_event_listener<video_group_change_request_event>([this](const video_group_change_request_event& event)
		{
			auto& main_player = ctx_.get_window<widgets::video_player>();
			if (&event.player() != &main_player) return;

			auto new_group_id = event.new_group_id();
			auto current_group_id = current_video_group_id_;

			const auto& video_groups = ctx_.current_project->video_groups;

			if (current_group_id == new_group_id) return;

			if (new_group_id != invalid_video_group_id and video_groups.find(new_group_id) == video_groups.end())
			{
				debug::error("Video group with id {} does not exist", new_group_id);
				return;
			}

			current_video_group_id_ = new_group_id;
			insert_segment_marks_.clear();
			selected_segments_.clear();
			dragged_segments_.clear();

			ctx_.dispatch_event<video_group_changed_event>(event.source(), main_player, current_group_id, new_group_id);
		});

		ctx_.add_event_listener<segment_insert_mark_start>([this](const segment_insert_mark_start& event)
		{
			insert_segment_marks_.push_back({ event.tag(), event.timestamp(), event.mark_id() });
		});

		ctx_.add_event_listener<segment_insert_mark_end>([this](const segment_insert_mark_end& event)
		{
			auto it = find_insert_segment_mark_by_id(event.mark_id());
			if (it == insert_segment_marks_.end()) return;

			ctx_.dispatch_event<segment_insert_request_event>(event.source(), event.storage(), it->tag, it->start, event.timestamp(), event.user_customization(), false);

			insert_segment_marks_.erase(it);
		});

		ctx_.add_event_listener<begin_segment_drag_event>([this](const begin_segment_drag_event& event)
		{
			if (is_dragging_any_segment()) return;

			dragged_segments_ = event.segments();

			segment_drag_data_.stage = segment_drag_stage::dragging;
			segment_drag_data_.grab_part = event.grab_part();
			segment_drag_data_.current_offset = timestamp::zero();
			segment_drag_data_.begin_drag_source = event.source();
		});

		ctx_.add_event_listener<update_segment_drag_event>([this](const update_segment_drag_event& event)
		{
			if (!is_dragging_any_segment()) return;

			segment_drag_data_.current_offset = event.current_offset();
		});

		ctx_.add_event_listener<end_segment_drag_event>([this](const end_segment_drag_event& e)
		{
			if (!is_dragging_any_segment()) return;

			segment_drag_data_.stage = segment_drag_stage::waiting_for_approval;
		});

		ctx_.add_event_listener<segments_moved_event>([this](const segments_moved_event& e)
		{
			if (segment_drag_data_.stage != segment_drag_stage::waiting_for_approval) return;

			segment_drag_data_ = vt::segment_drag_data{};
			dragged_segments_.clear();
		});

		ctx_.add_event_listener<segment_merged_event>([this](const segment_merged_event& event)
		{
			if (segment_id_map_erase(selected_segments_, event.tag(), event.merged_id()))
			{
				//ctx_.dispatch_event<segment_deselected_event>(event_source_, event.storage(), event.tag(), event.merged_id());
				selected_segments_[event.tag()].insert(event.merged_into_id());
				ctx_.dispatch_event<segment_select_request_event>(event_source_, event.storage(), event.tag(), event.merged_into_id());
			}

			if (segment_id_map_erase(dragged_segments_, event.tag(), event.merged_id()))
			{
				dragged_segments_[event.tag()].insert(event.merged_into_id());
			}
		});

		ctx_.add_event_listener<segment_deleted_event>([this](const segment_deleted_event& event)
		{
			if (!event.deleted()) return;

			segment_id_map_erase(selected_segments_, event.tag(), event.id());
			segment_id_map_erase(dragged_segments_, event.tag(), event.id());

			if (dragged_segments_.empty())
			{
				segment_drag_data_ = vt::segment_drag_data{};
				//ctx_.dispatch_event<end_segment_drag_event>(event_source_, event.storage(), segment_id_map{}, segment_part{}, timestamp::zero());
			}
		});

		ctx_.add_event_listener<tag_deleted_event>([this](const tag_deleted_event& e)
		{
			if (!e.deleted()) return;

			selected_segments_.erase(e.tag_name());
			dragged_segments_.erase(e.tag_name());

			if (dragged_segments_.empty())
			{
				segment_drag_data_ = vt::segment_drag_data{};
			}
		});

		ctx_.add_event_listener<tag_renamed_event>([this](const tag_renamed_event& e)
		{
			if (!e.renamed()) return;

			auto selected_it = selected_segments_.find(e.tag_name());
			if (selected_it != selected_segments_.end())
			{
				auto node = selected_segments_.extract(selected_it);
				node.key() = e.new_name();
				selected_segments_.insert(std::move(node));
			}

			auto dragged_it = dragged_segments_.find(e.tag_name());
			if (dragged_it != dragged_segments_.end())
			{
				auto node = dragged_segments_.extract(dragged_it);
				node.key() = e.new_name();
				dragged_segments_.insert(std::move(node));
			}
		});
	}

	const segment_id_map& session_storage::selected_segments() const
	{
		return selected_segments_;
	}

	const segment_id_map& session_storage::dragged_segments() const
	{
		return dragged_segments_;
	}

	const segment_drag_data& session_storage::segment_drag_data() const
	{
		return segment_drag_data_;
	}

	video_group_id_t session_storage::current_video_group_id() const
	{
		return current_video_group_id_;
	}

	const std::vector<insert_segment_mark_data>& session_storage::insert_segment_marks() const
	{
		return insert_segment_marks_;
	}

	bool session_storage::is_segment_selected(const std::string& tag, segment_id id) const
	{
		return segment_id_map_contains(selected_segments_, tag, id);
	}

	bool session_storage::is_any_segment_selected() const
	{
		return !selected_segments_.empty();
	}

	bool session_storage::more_than_one_segment_selected() const
	{
		size_t count = 0;
		for (const auto& [_, segments] : selected_segments_)
		{
			count += segments.size();
			if (count > 1) return true;
		}

		return false;
	}

	bool session_storage::is_segment_dragged(const std::string& tag, segment_id id) const
	{
		return segment_id_map_contains(dragged_segments_, tag, id);
	}

	bool session_storage::is_dragging_any_segment() const
	{
		return !dragged_segments_.empty();
	}

	std::vector<insert_segment_mark_data>::iterator session_storage::find_insert_segment_mark_by_tag(const std::string& tag)
	{
		return std::find_if(insert_segment_marks_.begin(), insert_segment_marks_.end(), [&](const auto& mark)
		{
			return mark.tag.has_value() and mark.tag.value() == tag;
		});
	}

	std::vector<insert_segment_mark_data>::iterator session_storage::find_insert_segment_mark_by_tag(const std::optional<std::string>& tag)
	{
		return std::find_if(insert_segment_marks_.begin(), insert_segment_marks_.end(), [&](const auto& mark)
		{
			return mark.tag == tag;
		});
	}

	std::vector<insert_segment_mark_data>::iterator session_storage::find_insert_segment_mark_by_id(uint64_t id)
	{
		return std::find_if(insert_segment_marks_.begin(), insert_segment_marks_.end(), [&](const auto& mark)
		{
			return mark.mark_id == id;
		});
	}

	void session_storage::reset()
	{
		selected_segments_.clear();
		dragged_segments_.clear();
		segment_drag_data_ = vt::segment_drag_data{};
		current_video_group_id_ = invalid_video_group_id;
		insert_segment_marks_.clear();
		tasks.clear();
	}
}
