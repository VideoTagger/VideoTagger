#include "session_storage.hpp"

#include <core/app_context.hpp>

#include <events/timeline/segment_select_request_event.hpp>
#include <events/timeline/segment_selected_event.hpp>
#include <events/timeline/segment_deselect_request_event.hpp>
#include <events/timeline/segment_deselected_event.hpp>
#include <events/timeline/segment_select_all_request_event.hpp>
#include <events/timeline/segment_deselect_all_request_event.hpp>
#include <events/timeline/segment_insert_mark_start.hpp>
#include <events/timeline/segment_insert_mark_end.hpp>
#include <events/timeline/begin_segment_drag_event.hpp>
#include <events/timeline/update_segment_drag_event.hpp>
#include <events/timeline/end_segment_drag_event.hpp>
#include <events/timeline/segments_moved_event.hpp>
#include <events/timeline/segment_merged_event.hpp>
#include <events/timeline/segment_deleted_event.hpp>
#include <events/timeline/segment_select_one_request_event.hpp>

#include <events/tags/tag_renamed_event.hpp>
#include <events/tags/tag_deleted_event.hpp>


#include <events/player/video_group_change_request_event.hpp>
#include <events/player/video_group_changed_event.hpp>
#include <events/player/seek_to_start_request_event.hpp>

#include <events/gizmo/gizmo_set_targets_event.hpp>
#include <events/gizmo/gizmo_move_targets_event.hpp>

#include <events/attributes/region_select_request_event.hpp>
#include <events/attributes/region_selected_event.hpp>
#include <events/attributes/region_deselect_request_event.hpp>
#include <events/attributes/region_deselected_event.hpp>
#include <events/attributes/region_hover_started_event.hpp>
#include <events/attributes/region_hover_ended_event.hpp>
#include <events/attributes/region_deleted_event.hpp>
#include <events/attributes/regions_track_cancel_request_event.hpp>
#include <events/attributes/regions_track_started_event.hpp>
#include <events/attributes/regions_track_ended_event.hpp>
#include <events/attributes/regions_track_request_event.hpp>
#include <events/attributes/attribute_deleted_event.hpp>
#include <events/attributes/attribute_renamed_event.hpp>
#include <events/attributes/attribute_instance_deleted_event.hpp>

namespace vt
{
	session_storage::session_storage() :
		tasks{ ctx_.tasks }, is_edit_mode_{}
	{
		register_timeline_listeners();
		register_gizmo_listeners();
		register_attribute_listeners();
	}

	void session_storage::register_timeline_listeners()
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

			ctx_.dispatch_event<segment_deselected_event>(event.source(), event.storage(), event.tag(), event.id());
		});

		ctx_.add_event_listener<segment_select_all_request_event>([this](const segment_select_all_request_event& event)
		{
			const auto& segment_strg = ctx_.get_current_segment_storage();
			if (&event.storage() != &segment_strg) return;

			for (const auto& [tag, segments] : segment_strg)
			{
				for (const auto& [id, segment] : segments)
				{
					auto [_, inserted] = selected_segments_[tag].insert(id);
					if (inserted)
					{
						ctx_.dispatch_event<segment_selected_event>(event.source(), event.storage(), tag, id);
					}
				}
			}
		});

		ctx_.add_event_listener<segment_select_one_request_event>([this](const segment_select_one_request_event& event)
		{
			const auto& segment_strg = ctx_.get_current_segment_storage();
			if (&event.storage() != &segment_strg) return;

			if (is_segment_selected(event.tag(), event.id()))
			{
				ctx_.dispatch_event<segment_deselect_all_request_event>(event.source(), event.storage(), segment_id_map{ { event.tag(), { event.id() } } });
			}
			else
			{
				ctx_.dispatch_event<segment_deselect_all_request_event>(event.source(), event.storage());
				ctx_.dispatch_event<segment_select_request_event>(event.source(), event.storage(), event.tag(), event.id());
			}
		});

		ctx_.add_event_listener<segment_deselect_all_request_event>([this](const segment_deselect_all_request_event& event)
		{
			const auto& segment_strg = ctx_.get_current_segment_storage();
			if (&event.storage() != &segment_strg) return;

			for (auto storage_it = selected_segments_.begin(); storage_it != selected_segments_.end();)
			{
				std::string tag = storage_it->first;
				auto& segments = storage_it->second;

				bool increment_storage_it = true;
				for (auto segment_it = segments.begin(); segment_it != segments.end();)
				{
					segment_id id = *segment_it;

					if (event.is_excluded(tag, id))
					{
						++segment_it;
						continue;
					}
					
					segment_it = segments.erase(segment_it);
					ctx_.dispatch_event<segment_deselected_event>(event.source(), event.storage(), tag, id);
					
					if (segment_it == segments.end())
					{
						storage_it = selected_segments_.erase(storage_it);
						increment_storage_it = false;
						break;
					}
				}

				if (increment_storage_it)
				{
					++storage_it;
				}
			}
		});

		ctx_.add_event_listener<segment_deselected_event>([this](const segment_deselected_event& event)
		{
			if (selected_region_.has_value())
			{
				if (selected_region_->tag_name == event.tag() and selected_region_->segment == event.id())
				{
					ctx_.dispatch_event<region_deselect_request_event>(event.source());
				}
			}
		});

		ctx_.add_event_listener<video_group_change_request_event>([this](const video_group_change_request_event& event)
		{
			auto& main_player = ctx_.get_window<widgets::video_player>();
			if (&event.player() != &main_player) return;

			if (ctx_.session.is_edit_mode() and event.new_group_id() != invalid_video_group_id) return;

			auto new_group_id = event.new_group_id();
			auto current_group_id = current_video_group_id_;

			const auto& video_groups = ctx_.current_project->video_groups;

			if (current_group_id == new_group_id) return;

			if (new_group_id != invalid_video_group_id and video_groups.find(new_group_id) == video_groups.end())
			{
				debug::error("Video group with id {} does not exist", new_group_id);
				return;
			}

			if (current_video_group_id_ != invalid_video_group_id)
			{
				ctx_.dispatch_event<segment_deselect_all_request_event>(event.source(), ctx_.get_current_segment_storage());
				ctx_.dispatch_event<region_deselect_request_event>(event.source());
			}

			current_video_group_id_ = new_group_id;
			insert_segment_marks_.clear();
			dragged_segments_.clear();

			ctx_.dispatch_event<video_group_changed_event>(event.source(), main_player, current_group_id, new_group_id);
			ctx_.dispatch_event<seek_to_start_request_event>(event.source(), main_player);
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

		ctx_.add_event_listener<end_segment_drag_event>([this](const end_segment_drag_event& event)
		{
			if (!is_dragging_any_segment()) return;

			segment_drag_data_.stage = segment_drag_stage::waiting_for_approval;
		});

		ctx_.add_event_listener<segments_moved_event>([this](const segments_moved_event& event)
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

		ctx_.add_event_listener<tag_deleted_event>([this](const tag_deleted_event& event)
		{
			if (!event.deleted()) return;

			selected_segments_.erase(event.tag_name());
			dragged_segments_.erase(event.tag_name());

			if (dragged_segments_.empty())
			{
				segment_drag_data_ = vt::segment_drag_data{};
			}
		});

		ctx_.add_event_listener<tag_renamed_event>([this](const tag_renamed_event& event)
		{
			if (!event.renamed()) return;

			auto selected_it = selected_segments_.find(event.tag_name());
			if (selected_it != selected_segments_.end())
			{
				auto node = selected_segments_.extract(selected_it);
				node.key() = event.new_name();
				selected_segments_.insert(std::move(node));
			}

			auto dragged_it = dragged_segments_.find(event.tag_name());
			if (dragged_it != dragged_segments_.end())
			{
				auto node = dragged_segments_.extract(dragged_it);
				node.key() = event.new_name();
				dragged_segments_.insert(std::move(node));
			}

			if (selected_region_.has_value())
			{
				if (selected_region_->tag_name == event.tag_name())
				{
					selected_region_->tag_name = event.new_name();
				}
			}

			if (tracked_regions_.has_value())
			{
				for (auto& region_data : tracked_regions_->region_data)
				{
					if (region_data.tag_name == event.tag_name())
					{
						region_data.tag_name = event.new_name();
					}
				}
			}

			for (auto& region : hovered_regions_)
			{
				if (region.tag_name == event.tag_name())
				{
					region.tag_name = event.new_name();
				}
			}
		});
	}

	void session_storage::register_gizmo_listeners()
	{
		ctx_.add_event_listener<gizmo_move_targets_event>([this](const gizmo_move_targets_event& event)
		{
			auto type = event.move_type();
			switch (type)
			{
				case gizmo_move_type::absolute:
				{
					for (const auto& target : gizmo_data_.targets)
					{
						target->at(0) = event.value().at(0);
						target->at(1) = event.value().at(1);
					}
					break;
				}
				case gizmo_move_type::offset:
				{
					for (const auto& target : gizmo_data_.targets)
					{
						target->at(0) += event.value().at(0);
						target->at(1) += event.value().at(1);
					}
					break;
				}
			}
			ctx_.is_project_dirty = true;
		});

		ctx_.add_event_listener<gizmo_set_targets_event>([this](const gizmo_set_targets_event& event)
		{
			gizmo_data_.targets = event.targets();
			gizmo_data_.video_id = gizmo_data_.targets.empty() ? video_id_t{} : event.video_id();
		});
	}

	void session_storage::register_attribute_listeners()
	{
		ctx_.add_event_listener<region_select_request_event>([this](const region_select_request_event& event)
		{
			region_info region_data{ event.tag_name(), event.segment(), event.video_id(), event.attribute_instance()->attribute_name(), event.attribute_instance(), event.region_id()};

			if (selected_region_.has_value())
			{
				bool is_already_selected = selected_region_->attribute_instance == event.attribute_instance() and selected_region_->region_id == event.region_id();
				if (is_already_selected) return;

				if (*selected_region_ == region_data)
				{
					ctx_.dispatch_event<segment_select_one_request_event>(event.source(), ctx_.get_current_segment_storage(), event.tag_name(), event.segment());
					return;
				}

				ctx_.dispatch_event<region_deselect_request_event>(event.source());
			}

			selected_region_ = region_data;

			ctx_.dispatch_event<region_selected_event>(event.source(), event.tag_name(), event.segment(), event.video_id(), event.attribute_instance(), event.region_id());
			
			ctx_.dispatch_event<segment_select_one_request_event>(event.source(), ctx_.get_current_segment_storage(), event.tag_name(), event.segment());
		});

		ctx_.add_event_listener<region_deselect_request_event>([this](const region_deselect_request_event& event)
		{
			if (!selected_region_.has_value()) return;

			ctx_.dispatch_event<region_deselected_event>(event.source(), selected_region_->tag_name, selected_region_->segment, selected_region_->video_id,
				selected_region_->attribute_instance, selected_region_->region_id);
			selected_region_.reset();
			gizmo_data_.targets.clear();
			gizmo_data_.video_id = 0;
		});

		ctx_.add_event_listener<region_hover_started_event>([this](const region_hover_started_event& event)
		{
			if (is_region_hovered(event.attribute_instance(), event.region_id())) return;

			hovered_regions_.push_back({ event.tag_name(), event.segment(), event.video_id(), event.attribute_instance()->attribute_name(), event.attribute_instance(), event.region_id()});
		});

		ctx_.add_event_listener<region_hover_ended_event>([this](const region_hover_ended_event& event)
		{
			remove_hovered_region(event.attribute_instance(), event.region_id());
		});

		ctx_.add_event_listener<region_deleted_event>([this](const region_deleted_event& event)
		{
			if (selected_region_.has_value())
			{
				if (selected_region_->attribute_instance == event.attribute_instance() and selected_region_->region_id == event.region_id())
				{
					ctx_.dispatch_event<region_deselect_request_event>(event.source());
				}
			}

			remove_hovered_region(event.attribute_instance(), event.region_id());
		});

		ctx_.add_event_listener<regions_track_request_event>([this](const regions_track_request_event& event)
		{
			struct track_data
			{
				region_info region;
				std::shared_ptr<impl::region_tracker> tracker;
				bool stopped = false;
			};

			auto progress = std::make_shared<float>(0.f);
			cancellation_token token;
			
			std::map<video_id_t, std::vector<track_data>> data_by_video;
			for (auto& region_data : event.regions())
			{
				auto& data = data_by_video[region_data.video_id];
				data.emplace_back(track_data{ region_data, region_data.attribute_instance->new_region_tracker() });
			}

			tracked_regions_.emplace();
			tracked_regions_->region_data = event.regions();
			tracked_regions_->progress = progress;

			for (auto& [video_id, data] : data_by_video)
			{
				auto task = ctx_.tasks.run([data, tracker_name = event.tracker(), video_id,
					timespan = event.track_span(), replace_keyframes = event.replace_keyframes(), progress](cancellation_token& cancel_token) mutable
				{
					auto video = ctx_.current_project->videos.get(video_id);
					if (video == nullptr) return;

					auto stream = video->video();
					if (!stream.is_open()) return;

					auto current_ts = timespan.start;

					image<image_pixel_format::rgb8> image(stream.width(), stream.height());
					if (!stream.update_frame(image, current_ts.total_nanoseconds, true)) return;
					current_ts = timestamp{ stream.current_frame()->timestamp() };

					bool all_trackers_stopped = true;
					for (auto& region_data : data)
					{
						auto segment = ctx_.find_segment(region_data.region.tag_name, region_data.region.segment);
						if (segment == nullptr)
						{
							region_data.stopped = true;
							continue;
						}

						auto clamped_timespan = utils::timestamp_span{ std::clamp(timespan.start, segment->start, segment->end), std::clamp(timespan.end, segment->start, segment->end) };
						if (clamped_timespan.start >= clamped_timespan.end)
						{
							region_data.stopped = true;
							continue;
						}

						if (!region_data.tracker->init(region_data.region, tracker_name, clamped_timespan, image, replace_keyframes))
						{
							region_data.stopped = true;
						}
						else
						{
							all_trackers_stopped = false;
						}
					}

					if (all_trackers_stopped) return;

					do
					{
						all_trackers_stopped = true;

						if (cancel_token.is_cancelled()) break;

						//*progress = region_tracker->progress();

						current_ts = timestamp{ stream.current_frame()->next_timestamp() };
						if (!stream.update_frame(image, current_ts.total_nanoseconds, true)) break;
						current_ts = timestamp{ stream.current_frame()->timestamp() };

						for (auto& region_data : data)
						{
							if (region_data.stopped) continue;

							if (region_data.tracker->update(current_ts, image))
							{
								region_data.stopped = true;
							}
							else
							{
								all_trackers_stopped = false;
							}
						}

					} while (!stream.eof() and !all_trackers_stopped);

				}, token);

				task.then(ctx_.tasks.on_main(), [data, source = event.source()](cancellation_token& cancel_token)
				{
					std::vector<region_info> regions;
					for (auto& region_data : data)
					{
						region_data.tracker->finalize(true);
						regions.push_back(region_data.region);
					}

					ctx_.dispatch_event<regions_track_ended_event>(source, regions);
				}, token);

				tracked_regions_->tasks.emplace_back(std::move(task));
			}

			
			ctx_.dispatch_event<regions_track_started_event>(event.source(), event.regions());

			ctx_.global_progress_popup = std::make_unique<ui::progress_popup>
			(
				"Tracking",
				[data_by_video](ui::progress_popup&)
				{
					size_t count{};
					float progress{};

					for (auto& [_, data] : data_by_video)
					{
						for (auto& tracker : data)
						{
							progress += tracker.stopped ? 1.f : tracker.tracker->progress();
						}

						count += data.size();
					}

					return progress / count;
				},
				[](const std::optional<float>& progress)
				{
					return *progress >= 1.f or !ctx_.session.is_any_region_tracked();
				},
				[source = event.source()]()
				{
					ctx_.dispatch_event<regions_track_cancel_request_event>(source);
				}
			);
		});

		ctx_.add_event_listener<regions_track_ended_event>([this](const regions_track_ended_event& event)
		{
			if (!tracked_regions_.has_value()) return;

			for (auto& region : event.regions())
			{
				auto it = std::find_if(tracked_regions_->region_data.begin(), tracked_regions_->region_data.end(), [id = region.region_id](const region_info& region)
				{
					return id == region.region_id;
				});

				if (it != tracked_regions_->region_data.end())
				{
					tracked_regions_->region_data.erase(it);
				}
			}

			if (tracked_regions_->region_data.empty())
			{
				tracked_regions_.reset();
			}
		});

		ctx_.add_event_listener<regions_track_cancel_request_event>([this](const regions_track_cancel_request_event& event)
		{
			if (!tracked_regions_.has_value()) return;

			for (auto& task : tracked_regions_->tasks)
			{
				task.token().cancel();
			}

			tracked_regions_.reset();
		});

		ctx_.add_event_listener<attribute_deleted_event>([this](const attribute_deleted_event& event)
		{
			if (selected_region_.has_value())
			{
				if (selected_region_->attribute_name == event.attribute_name())
				{
					selected_region_.reset();
				}
			}

			tracked_regions_.reset();

			for (auto it = hovered_regions_.begin(); it != hovered_regions_.end();)
			{
				if (it->attribute_name == event.attribute_name())
				{
					it = hovered_regions_.erase(it);
					continue;
				}

				it++;
			}
		});

		ctx_.add_event_listener<attribute_renamed_event>([this](const attribute_renamed_event& event)
		{
			if (selected_region_.has_value())
			{
				if (selected_region_->attribute_name == event.attribute_name())
				{
					selected_region_->attribute_name = event.attribute_name();
				}
			}

			if (tracked_regions_.has_value())
			{
				for (auto& region_data : tracked_regions_->region_data)
				{
					if (region_data.attribute_name == event.attribute_name())
					{
						region_data.attribute_name = event.attribute_name();
					}
				}
			}

			for (auto& region_data : hovered_regions_)
			{
				if (region_data.attribute_name != event.attribute_name()) continue;

				region_data.attribute_name = event.attribute_name();
			}
		});

		ctx_.add_event_listener<attribute_instance_deleted_event>([this](const attribute_instance_deleted_event& event)
		{
			if (selected_region_.has_value())
			{
				if (selected_region_->attribute_instance == event.attribute_instance())
				{
					ctx_.dispatch_event<region_deselect_request_event>(event.source());
				}
			}

			tracked_regions_.reset();

			for (auto it = hovered_regions_.begin(); it != hovered_regions_.end();)
			{
				if (it->attribute_instance == event.attribute_instance())
				{
					it = hovered_regions_.erase(it);
					continue;
				}
				++it;
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

	bool session_storage::is_any_video_group_active() const
	{
		return current_video_group_id_ != invalid_video_group_id;
	}

	const std::vector<insert_segment_mark_data>& session_storage::insert_segment_marks() const
	{
		return insert_segment_marks_;
	}

	const std::optional<region_info>& session_storage::selected_region() const
	{
		return selected_region_;
	}

	bool session_storage::is_region_selected(impl::attribute_instance* attribute_instance, region_id_t region_id) const
	{
		if (!selected_region_.has_value()) return false;

		return selected_region_->attribute_instance == attribute_instance and selected_region_->region_id == region_id;
	}

	bool session_storage::is_any_region_selected() const
	{
		return selected_region_.has_value();
	}

	const std::vector<region_info>& session_storage::hovered_regions() const
	{
		return hovered_regions_;
	}

	bool session_storage::is_region_hovered(impl::attribute_instance* attribute_instance, region_id_t region_id) const
	{
		auto it = std::find_if(hovered_regions_.begin(), hovered_regions_.end(), [attribute_instance, region_id](const region_info& region_data)
		{
			return region_data.attribute_instance == attribute_instance and region_data.region_id == region_id;
		});
		return it != hovered_regions_.end();
	}

	bool session_storage::is_any_region_hovered() const
	{
		return !hovered_regions_.empty();
	}

	const std::optional<tracked_regions_data>& session_storage::tracked_regions() const
	{
		return tracked_regions_;
	}

	bool session_storage::is_region_tracked(impl::attribute_instance* attribute_instance, region_id_t region_id) const
	{
		if (!tracked_regions_.has_value()) return false;

		for (auto& region_data : tracked_regions_->region_data)
		{
			if (region_data.attribute_instance == attribute_instance and region_data.region_id == region_id) return true;
		}

		return false;
	}

	bool session_storage::is_any_region_tracked() const
	{
		return tracked_regions_.has_value();
	}

	bool session_storage::is_segment_selected(const std::string& tag, segment_id id) const
	{
		return segment_id_map_contains(selected_segments_, tag, id);
	}

	bool session_storage::is_one_segment_selected() const
	{
		return is_any_segment_selected() and !is_more_than_one_segment_selected();
	}

	bool session_storage::is_any_segment_selected() const
	{
		return !selected_segments_.empty();
	}

	bool session_storage::is_more_than_one_segment_selected() const
	{
		size_t count = 0;
		for (const auto& [_, segments] : selected_segments_)
		{
			count += segments.size();
			if (count > 1) return true;
		}

		return false;
	}

	std::optional<std::pair<std::string, segment_id>> session_storage::any_selected_segment() const
	{
		for (auto& [tag, seg] : selected_segments_)
		{
			if (!seg.empty())
			{
				return std::make_pair(tag, *seg.begin());
			}
		}

		return std::nullopt;
	}

	std::optional<segment_id> session_storage::any_selected_segment(const std::string& tag) const
	{
		auto it = selected_segments_.find(tag);
		if (it == selected_segments_.end()) return std::nullopt;

		const auto& segments = it->second;
		if (segments.empty()) return std::nullopt;

		return *segments.begin();
	}

	bool session_storage::is_segment_dragged(const std::string& tag, segment_id id) const
	{
		return segment_id_map_contains(dragged_segments_, tag, id);
	}

	bool session_storage::is_dragging_any_segment() const
	{
		return !dragged_segments_.empty();
	}

	bool session_storage::gizmo_contains_target(const utils::vec2<int>* target) const
	{
		auto it = std::find_if(gizmo_data_.targets.begin(), gizmo_data_.targets.end(), [&](const auto& gizmo_target)
		{
			return gizmo_target == target;
		});
		return it != gizmo_data_.targets.end();
	}

	video_id_t session_storage::gizmo_video_id() const
	{
		return gizmo_data_.video_id;
	}

	const std::vector<utils::vec2<int>*>& session_storage::gizmo_targets() const
	{
		return gizmo_data_.targets;
	}

	bool session_storage::has_gizmo_targets() const
	{
		return !gizmo_data_.targets.empty();
	}

	utils::vec2<int> session_storage::mean_gizmo_target() const
	{
		utils::vec2<int> result;
		for (const auto& target : gizmo_data_.targets)
		{
			result[0] += target->at(0);
			result[1] += target->at(1);
		}

		result[0] /= gizmo_data_.targets.size();
		result[1] /= gizmo_data_.targets.size();
		return result;
	}

	void session_storage::set_edit_mode(bool value)
	{
		is_edit_mode_ = value;
	}

	bool session_storage::is_edit_mode() const
	{
		return is_edit_mode_;
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
		selected_region_.reset();
		tracked_regions_.reset();
		hovered_regions_.clear();
		
		gizmo_data_.video_id = 0;
		gizmo_data_.targets.clear();
		toolbar.reset();
		tasks.clear();
		is_edit_mode_ = false;
	}

	bool session_storage::remove_hovered_region(impl::attribute_instance* attribute_instance, region_id_t region_id)
	{
		auto it = std::find_if(hovered_regions_.begin(), hovered_regions_.end(), [attribute_instance, region_id](const region_info& region_data)
		{
			return region_data.attribute_instance == attribute_instance and region_data.region_id == region_id;
		});
		if (it == hovered_regions_.end()) return false;

		hovered_regions_.erase(it);
		return true;
	}
}
