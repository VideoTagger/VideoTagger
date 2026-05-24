#include "pch.hpp"
#include "video_player.hpp"
#include <utils/timestamp.hpp>
#include <core/debug.hpp>
#include "slider.hpp"
#include "controls.hpp"
#include <ui/icons.hpp>
#include <ui/widgets/common.hpp>
#include "time_input.hpp"
#include <core/app_context.hpp>
#include <events/player/playback_suspend_request_event.hpp>
#include <events/player/playback_resume_request_event.hpp>
#include <events/player/playback_change_request_event.hpp>
#include <events/player/playback_changed_event.hpp>
#include <events/player/looping_changed_event.hpp>
#include <events/player/looping_change_request_event.hpp>
#include <events/player/seek_request_event.hpp>
#include <events/player/seek_event.hpp>
#include <events/player/seek_to_start_request_event.hpp>
#include <events/player/seek_to_end_request_event.hpp>
#include <events/player/seek_to_previous_frame_request_event.hpp>
#include <events/player/seek_to_next_frame_request_event.hpp>
#include <events/player/skip_next_request_event.hpp>
#include <events/player/skip_previous_request_event.hpp>
#include <events/player/speed_changed_event.hpp>
#include <events/player/speed_change_request_event.hpp>


namespace vt::widgets
{
	static size_t to_power_of2(size_t n)
	{
		if (n <= 1) return 1;
		return static_cast<size_t>(std::pow(2, std::ceil(std::log2(n))));
	}

	video_player::video_player() : ui::window{ "Video Player", "video-player", "Video Player", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse },
		progress_{}, dock_window_count_{}, speed_{ 1.0f }, is_playing_{}, autoplay_{}, loop_mode_{}, show_video_ids_{}
	{
		set_icon(icons::play);

		ctx_.add_event_listener<playback_suspend_request_event>([this](const playback_suspend_request_event& event)
		{
			if (&event.player() != this) return;

			if (!is_playing_ or playback_suspend_source_.has_value()) return;

			playback_suspend_source_ = event.source();
			
			ctx_.dispatch_event<playback_change_request_event>(event.source(), *this, false);
		});

		ctx_.add_event_listener<playback_resume_request_event>([this](const playback_resume_request_event& event)
		{
			if (&event.player() != this) return;

			if (is_playing_ or !playback_suspend_source_.has_value() or playback_suspend_source_ != event.source()) return;

			playback_suspend_source_ = std::nullopt;
			
			ctx_.dispatch_event<playback_change_request_event>(event.source(), *this, true);
		});

		ctx_.add_event_listener<playback_changed_event>([this](const playback_changed_event& event)
		{
			if (&event.player() != this) return;

			if (playback_suspend_source_ != event.source())
			{
				playback_suspend_source_ = std::nullopt;
			}
			set_playing(event.is_playing());
		});

		ctx_.add_event_listener<looping_changed_event>([this](const looping_changed_event& event)
		{
			if (&event.player() != this) return;

			set_loop_mode(event.mode());
		});

		ctx_.add_event_listener<speed_changed_event>([this](const speed_changed_event& event)
		{
			if (&event.player() != this) return;

			speed_ = event.speed();
		});

		ctx_.add_event_listener<seek_event>([this](const seek_event& event)
		{
			if (&event.player() != this) return;
			
			data_.current_ts = event.timestamp();
		});

		progress_.set_tooltip_enabled(false);
		progress_.set_step(1);
		progress_.set_on_change_callback([this](int64_t old_value, int64_t new_value)
		{
			if (old_value == new_value) return;
			auto new_ts = std::chrono::nanoseconds{ new_value };

			ctx_.dispatch_event<seek_request_event>(get_event_source(), *this, new_ts);
			std::invoke(callbacks.on_seek, new_ts);
		});
	}

	void video_player::update_data(video_player_data data, bool is_playing)
	{
		data_ = data;
		is_playing_ = is_playing;

		if (data.current_ts == data.end_ts and callbacks.on_finish)
		{
			callbacks.on_finish(loop_mode_, is_playing_);
		}
	}

	void video_player::reset_data()
	{
		progress_.set_value(0);
		data_.current_ts = {};
		data_.start_ts = {};
		data_.end_ts = {};
		is_playing_ = false;
	}

	void video_player::dock_windows(size_t count)
	{
		dock_window_count_ = count;
	}

	bool video_player::prepare_video_windows(size_t count)
	{
		if (count == video_windows_.size()) return false;

		video_windows_.clear();
		for (uint64_t i = 0; i < count; ++i)
		{
			video_windows_.push_back(std::make_unique<ui::windows::video_window>(i));
		}
		return true;
	}

	const video_player_data& video_player::data() const
	{
		return data_;
	}

	void video_player::set_loop_mode(vt::loop_mode value)
	{
		loop_mode_ = value;
	}

	void video_player::set_show_video_ids(bool value)
	{
		show_video_ids_ = value;
	}

	void video_player::set_playing(bool value)
	{
		is_playing_ = value;
		std::invoke(callbacks.on_set_playing, is_playing_);
	}
	
	bool video_player::is_playing() const
	{
		return is_playing_;
	}

    bool video_player::should_autoplay() const
    {
        return autoplay_;
    }

	loop_mode video_player::loop_mode() const
	{
		return loop_mode_;
	}

	bool video_player::show_video_ids() const
	{
		return show_video_ids_;
	}

	std::vector<std::unique_ptr<ui::windows::video_window>>& video_player::video_windows()
	{
		return video_windows_;
	}

	nlohmann::ordered_json video_player::serialize() const
	{
		nlohmann::ordered_json json;
		json["autoplay"] = autoplay_;
		return json;
	}

	void video_player::deserialize(const nlohmann::ordered_json& json)
	{
		if (json.contains("autoplay") and json["autoplay"].is_boolean())
		{
			autoplay_ = json["autoplay"].get<bool>();
		}
	}

	void video_player::pre_style()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
	}

	void video_player::post_style()
	{
		ImGui::PopStyleVar();
	}

	void video_player::on_render()
	{
		auto& io = ImGui::GetIO();
		auto& style = ImGui::GetStyle();
		bool has_child_videos = data_.start_ts != data_.end_ts;

		float button_size = 25 * io.FontGlobalScale;

		auto& imgui_style = ImGui::GetStyle();
		auto image_avail_size = ImGui::GetContentRegionAvail();
		auto text_height = ImGui::GetTextLineHeightWithSpacing();
		image_avail_size.y -= button_size + 2 * imgui_style.ItemSpacing.y + text_height * io.FontGlobalScale + 2 * style.FramePadding.y;

		if (ImGui::BeginChild("##VideoPlayerFrame", image_avail_size))
		{
			if (dock_window_count_ > 0)
			{
				ImGuiID dock_node_id = ImGui::GetID("##VideoPlayerFrameDockspace");
				auto node = ImGui::DockBuilderGetNode(dock_node_id);
				if (node != nullptr)
				{
					debug::log("Redocking videos...");

					auto node_size = node->Size;
					ImGui::DockBuilderRemoveNode(dock_node_id);
					auto dockspace_flags = ImGuiDockNodeFlags_AutoHideTabBar | ImGuiDockNodeFlags_PassthruCentralNode;
					dock_node_id = ImGui::DockBuilderAddNode(dock_node_id, dockspace_flags);
					ImGui::DockBuilderSetNodeSize(dock_node_id, node_size);

					ImGuiID temp_id = dock_node_id;
					auto row_n = std::ceil(std::log2(dock_window_count_));
					if (row_n <= 1)
					{
						row_n = 1;
					}
					size_t rows = static_cast<size_t>(row_n);
					size_t columns = (dock_window_count_ + rows - 1) / std::max(rows, static_cast<size_t>(1));

					for (size_t y = 0; y < rows; ++y)
					{
						ImGuiID lower_node{};
						temp_id = ImGui::DockBuilderSplitNode(temp_id, ImGuiDir_Up, 1.0f / rows, nullptr, &lower_node);

						for (size_t x = 0; x < columns; ++x)
						{
							size_t id = y * rows + x;
							auto video_id = "###video-window-" + std::to_string(id);

							ImGui::DockBuilderSplitNode(temp_id, ImGuiDir_Right, 1.0f / columns, &temp_id, nullptr);
							ImGui::DockBuilderDockWindow(video_id.c_str(), temp_id);
						}
						temp_id = lower_node;
					}
					ImGui::DockBuilderFinish(dock_node_id);
					dock_window_count_ = 0;
				}
			}
			auto flags = ImGuiDockNodeFlags_AutoHideTabBar | ImGuiDockNodeFlags_PassthruCentralNode;
			ImGui::DockSpace(ImGui::GetID("##VideoPlayerFrameDockspace"), ImGui::GetContentRegionAvail(), flags);
		}
		ImGui::EndChild();

		timestamp current_time{ std::chrono::duration_cast<std::chrono::milliseconds>(data_.current_ts) };
		timestamp duration{ std::chrono::duration_cast<std::chrono::milliseconds>(data_.end_ts) };
		decltype(data_.current_ts) min_ts{};

		ImGui::BeginGroup();
		auto progress_size = ImVec2{ ImGui::GetContentRegionAvail().x, text_height };
		if (has_child_videos)
		{
			//if (slider_scalar("##VideoProgressBar", ImGuiDataType_U64, progress_size, text_height / 5.f, &data_.current_ts, &min_ts, &data_.end_ts, "", ImGuiSliderFlags_AlwaysClamp))
			//{
			//	ctx_.dispatch_event<seek_request_event>(get_event_source(), *this, data_.current_ts);
			//	std::invoke(callbacks.on_seek, data_.current_ts);
			//}

			progress_.set_value(data_.current_ts.count(), false);
			progress_.set_size(progress_size);
			progress_.set_range(min_ts.count(), data_.end_ts.count());
			progress_.render();
		}
		else
		{
			ImGui::Dummy(progress_size);
		}

		if (!has_child_videos) ImGui::BeginDisabled();
		ImGui::Columns(3);
		{
			int64_t frame{};
			static bool is_frame_dragging = false;
			if (callbacks.on_seek != nullptr and frame_dragger(frame, is_frame_dragging))
			{
				if (callbacks.on_set_playing != nullptr)
				{
					callbacks.on_set_playing(false);
				}

				if (frame < 0)
				{
					ctx_.dispatch_event<seek_to_previous_frame_request_event>(get_event_source(), *this);
				}
				else if (frame > 0)
				{
					ctx_.dispatch_event<seek_to_next_frame_request_event>(get_event_source(), *this);
				}
			}
			ImGui::SameLine();

			auto avail_size = ImGui::GetContentRegionAvail();
			auto time_size = ImGui::CalcTextSize("00:00:00:000");

			ImGui::SetNextItemWidth(time_size.x);
			ImGui::PushStyleColor(ImGuiCol_FrameBg, { 0, 0, 0, 0 });
			if (widgets::time_input("##TimeInput", &current_time, 1'000'000, 0, duration.total_nanoseconds.count()))
			{
				ctx_.dispatch_event<seek_request_event>(get_event_source(), *this, current_time.total_nanoseconds);
				callbacks.on_seek(current_time.total_nanoseconds);
			}
			ImGui::PopStyleColor();
			ImGui::SameLine();

			std::string video_duration = fmt::format("| {}", timestamp_to_string(duration, default_time_format));
			ImGui::TextUnformatted(video_duration.c_str());
		}

		ImGui::NextColumn();
		{
			auto avail_size = ImGui::GetContentRegionAvail();
			auto cursor_pos = ImGui::GetCursorPos();
			auto button_pos_x = avail_size.x / 2 - (button_size + imgui_style.ItemSpacing.x) * 5.f / 2;

			ImGui::SetCursorPosX(cursor_pos.x + button_pos_x);
			if (ui::icon_button(icons::skip_prev, { button_size, button_size }))
			{
				ctx_.dispatch_event<skip_previous_request_event>(get_event_source(), *this);
				std::invoke(callbacks.on_skip, -1, loop_mode_, is_playing_);
			}
			ImGui::SameLine();
			if (ui::icon_button(icons::fast_back, { button_size, button_size }))
			{
				ctx_.dispatch_event<seek_to_start_request_event>(get_event_source(), *this);
				std::invoke(callbacks.on_seek, std::chrono::nanoseconds{});
			}
			ImGui::SameLine();
			if (ui::icon_button(is_playing_ ? icons::pause : icons::play, { button_size, button_size }))
			{
				ctx_.dispatch_event<playback_change_request_event>(get_event_source(), *this, !is_playing_);
			}
			ImGui::SameLine();
			if (ui::icon_button(icons::fast_fwd, { button_size, button_size }))
			{
				ctx_.dispatch_event<seek_to_end_request_event>(get_event_source(), *this);
				std::invoke(callbacks.on_seek, std::chrono::nanoseconds(data_.end_ts));
			}
			ImGui::SameLine();
			if (ui::icon_button(icons::skip_next, { button_size, button_size }))
			{
				ctx_.dispatch_event<skip_next_request_event>(get_event_source(), *this);
				std::invoke(callbacks.on_skip, 1, loop_mode_, is_playing_);
			}
		}

		ImGui::NextColumn();
		{
			if (ui::icon_toggle_button(icons::autoplay, autoplay_, { button_size, button_size }))
			{
				autoplay_ = !autoplay_;
			}
			if (has_child_videos)
			{
				ui::tooltip(autoplay_ ? "Autoplay: On" : "Autoplay: Off");
			}

			ImGui::SameLine();
			bool looping = loop_mode_ != loop_mode::off;

			auto loop_icon = loop_mode_ != loop_mode::one ? icons::repeat : icons::repeat_one;
			if (ui::icon_toggle_button(loop_icon, looping, { button_size, button_size }))
			{
				switch (loop_mode_)
				{
					case loop_mode::off: loop_mode_ = loop_mode::all; break;
					case loop_mode::all: loop_mode_ = loop_mode::one; break;
					case loop_mode::one: loop_mode_ = loop_mode::off; break;
				}

				ctx_.dispatch_event<looping_change_request_event>(get_event_source(), *this, loop_mode_);
				std::invoke(callbacks.on_set_looping, loop_mode_);
			}
			if (has_child_videos)
			{
				switch (loop_mode_)
				{
					case loop_mode::off: ui::tooltip("Loop: Off"); break;
					case loop_mode::all: ui::tooltip("Loop: All"); break;
					case loop_mode::one: ui::tooltip("Loop: One"); break;
				}
			}
			ImGui::SameLine();

			auto avail_size = ImGui::GetContentRegionAvail();
			auto speed_control_size = ImVec2{ avail_size.x * 0.5f, ImGui::GetTextLineHeight() * io.FontGlobalScale + style.FramePadding.y * 2.f };

			static constexpr float speed_step = 0.25f;
			static constexpr float min_speed = speed_step;
			static constexpr float max_speed = 8.0f;

			ImGui::SetNextItemWidth(speed_control_size.x);
			if (ImGui::DragFloat("##VideoPlayerSpeed", &speed_, 0.1f, min_speed, max_speed, "%.2fx", ImGuiSliderFlags_AlwaysClamp))
			{
				ctx_.dispatch_event<speed_change_request_event>(get_event_source(), *this, speed_);
				if (callbacks.on_set_speed != nullptr)
				{
					std::invoke(callbacks.on_set_speed, speed_);
				}
			}

			if (ImGui::IsItemHovered() and io.MouseWheel != 0)
			{
				auto scroll_dir = !std::signbit(io.MouseWheel) * 2 - 1;
				speed_ = std::clamp(speed_ + scroll_dir * speed_step, min_speed, max_speed);
				ctx_.dispatch_event<speed_change_request_event>(get_event_source(), *this, speed_);
				if (callbacks.on_set_speed != nullptr) std::invoke(callbacks.on_set_speed, speed_);
			}

			if (ImGui::BeginPopupContextItem("##VideoPlayerSpeedCtx"))
			{
				if (ImGui::MenuItem("Reset"))
				{
					speed_ = 1.0f;
					ctx_.dispatch_event<speed_change_request_event>(get_event_source(), *this, speed_);
					if (callbacks.on_set_speed != nullptr) std::invoke(callbacks.on_set_speed, speed_);
				}
				ImGui::EndPopup();
			}

			//TODO: Maybe expose number of speeds in options
			size_t speed_option_count = 8;
			float popup_height = (speed_control_size.y + style.ItemSpacing.y) * speed_option_count + style.WindowPadding.y * 2.f;
			if (widgets::begin_button_dropdown("##VideoPlayerSpeedDropdown", speed_control_size, popup_height))
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{});
				for (size_t i = 0; i < speed_option_count; ++i)
				{
					float new_speed = speed_step * (i + 1);
					std::stringstream ss;
					ss << std::setprecision(3) << new_speed << 'x';
					std::string speed_str = (i + 1 == speed_option_count / 2) ? "Normal" : ss.str();
					bool disabled = (speed_ == new_speed);
					if (disabled) ImGui::BeginDisabled();
					if (ImGui::Button(speed_str.c_str(), speed_control_size))
					{
						speed_ = new_speed;
						ctx_.dispatch_event<speed_change_request_event>(get_event_source(), *this, speed_);
						if (callbacks.on_set_speed != nullptr) std::invoke(callbacks.on_set_speed, speed_);
						ImGui::CloseCurrentPopup();
					}
					if (disabled) ImGui::EndDisabled();
				}
				ImGui::PopStyleColor();
				widgets::end_button_dropdown();
			}
		}
		if (!has_child_videos) ImGui::EndDisabled();
		ImGui::EndGroup();
		ImGui::Columns();
	}
}
