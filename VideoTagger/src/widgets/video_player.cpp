#include "pch.hpp"
#include "video_player.hpp"
#include <utils/timestamp.hpp>
#include <core/debug.hpp>
#include "slider.hpp"
#include "controls.hpp"
#include "icons.hpp"
#include "time_input.hpp"
#include <core/app_context.hpp>

namespace vt::widgets
{
	static size_t to_power_of2(size_t n)
	{
		if (n <= 1) return 1;
		return static_cast<size_t>(std::pow(2, std::ceil(std::log2(n))));
	}

	video_player::video_player() : dock_window_count_{}, speed_{ 1.0f }, is_visible_{}, is_playing_ {}, loop_mode_{} {}

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
		data_.current_ts = {};
		data_.start_ts = {};
		data_.end_ts = {};
		is_playing_ = false;
	}

	void video_player::render()
	{
		auto& io = ImGui::GetIO();
		auto& style = ImGui::GetStyle();
		bool has_child_videos = data_.start_ts != data_.end_ts;
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

		float button_size = 25 * io.FontGlobalScale;
		std::string title = "Video Player";

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
		is_visible_ = ImGui::Begin(title.c_str(), nullptr, flags);
		ImGui::PopStyleVar();

		if (is_visible_)
		{
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
								auto video_id = "Video##" + std::to_string(id);

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
				if (slider_scalar("##VideoProgressBar", ImGuiDataType_U64, progress_size, text_height / 5.f, &data_.current_ts, &min_ts, &data_.end_ts, "", ImGuiSliderFlags_AlwaysClamp))
				{
					std::invoke(callbacks.on_seek, data_.current_ts);
				}
			}
			else
			{
				ImGui::Dummy(progress_size);
			}

			if (!has_child_videos) ImGui::BeginDisabled();
			ImGui::Columns(3);
			{
				auto avail_size = ImGui::GetContentRegionAvail();
				auto time_size = ImGui::CalcTextSize("00:00:00:000");
				auto total_size = ImGui::CalcTextSize("00:00:00:000 | 00:00:00:000");

				ImGui::SetCursorPos({ avail_size.x - total_size.x, ImGui::GetCursorPosY() + total_size.y / 4 });
				ImGui::SetNextItemWidth(time_size.x);
				ImGui::PushStyleColor(ImGuiCol_FrameBg, { 0, 0, 0, 0 });
				if (widgets::time_input("##TimeInput", &current_time, 1, 0, duration.total_milliseconds.count()))
				{
					callbacks.on_seek(current_time.total_milliseconds);
				}
				ImGui::PopStyleColor();
				ImGui::SameLine();

				std::string video_duration = fmt::format("| {}", utils::time::time_to_string(duration.total_milliseconds.count()));
				ImGui::TextUnformatted(video_duration.c_str());
			}

			ImGui::NextColumn();
			{
				auto avail_size = ImGui::GetContentRegionAvail();
				auto cursor_pos = ImGui::GetCursorPos();
				auto button_pos_x = avail_size.x / 2 - (button_size + imgui_style.ItemSpacing.x) * 5.f / 2;

				ImGui::SetCursorPosX(cursor_pos.x + button_pos_x);
				if (icon_button(icons::skip_prev, { button_size, button_size })) 
				{
					std::invoke(callbacks.on_skip, -1, loop_mode_, is_playing_);
				}
				ImGui::SameLine();
				if (icon_button(icons::fast_back, { button_size, button_size }))
				{
					set_playing(false);
					std::invoke(callbacks.on_seek, std::chrono::nanoseconds{});
				}
				ImGui::SameLine();
				if (icon_button(is_playing_ ? icons::pause : icons::play, { button_size, button_size }))
				{
					set_playing(!is_playing_);
				}
				ImGui::SameLine();
				if (icon_button(icons::fast_fwd, { button_size, button_size }))
				{
					set_playing(false);
					std::invoke(callbacks.on_seek, std::chrono::nanoseconds(data_.end_ts));
				}
				ImGui::SameLine();
				if (icon_button(icons::skip_next, { button_size, button_size }))
				{
					std::invoke(callbacks.on_skip, 1, loop_mode_, is_playing_);
				}
			}

			ImGui::NextColumn();
			{
				if (icon_toggle_button(icons::play_next, ctx_.app_settings.autoplay, { button_size, button_size }))
				{
					ctx_.app_settings.autoplay = !ctx_.app_settings.autoplay;
					ctx_.settings["autoplay"] = ctx_.app_settings.autoplay;
				}
				if (has_child_videos)
				{
					tooltip(ctx_.app_settings.autoplay ? "Autoplay: On" : "Autoplay: Off");
				}

				ImGui::SameLine();
				bool looping = loop_mode_ != loop_mode::off;

				auto loop_icon = loop_mode_ != loop_mode::one ? icons::repeat : icons::repeat_one;
				if (icon_toggle_button(loop_icon, looping, { button_size, button_size }))
				{
					switch (loop_mode_)
					{
					case loop_mode::off: loop_mode_ = loop_mode::all; break;
					case loop_mode::all: loop_mode_ = loop_mode::one; break;
					case loop_mode::one: loop_mode_ = loop_mode::off; break;
					}

					std::invoke(callbacks.on_set_looping, loop_mode_);
				}
				if (has_child_videos)
				{
					switch (loop_mode_)
					{
					case loop_mode::off: tooltip("Loop: Off"); break;
					case loop_mode::all: tooltip("Loop: All"); break;
					case loop_mode::one: tooltip("Loop: One"); break;
					}
				}
				ImGui::SameLine();

				auto avail_size = ImGui::GetContentRegionAvail();
				auto speed_control_size = ImVec2{ avail_size.x * 0.5f, ImGui::GetTextLineHeight() * io.FontGlobalScale + style.FramePadding.y * 2.f};

				static constexpr float speed_step = 0.25f;
				static constexpr float min_speed = speed_step;
				static constexpr float max_speed = 8.0f;

				ImGui::SetNextItemWidth(speed_control_size.x);
				if (ImGui::DragFloat("##VideoPlayerSpeed", &speed_, 0.1f, min_speed, max_speed, "%.2fx", ImGuiSliderFlags_AlwaysClamp) and callbacks.on_set_speed != nullptr)
				{
					std::invoke(callbacks.on_set_speed, speed_);
				}
				
				if (ImGui::IsItemHovered() and io.MouseWheel != 0)
				{
					auto scroll_dir = !std::signbit(io.MouseWheel) * 2 - 1;
					speed_ = std::clamp(speed_ + scroll_dir * speed_step, min_speed, max_speed);
					if (callbacks.on_set_speed != nullptr) std::invoke(callbacks.on_set_speed, speed_);
				}

				if (ImGui::BeginPopupContextItem("##VideoPlayerSpeedCtx"))
				{
					if (ImGui::MenuItem("Reset"))
					{
						speed_ = 1.0f;
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
		ImGui::End();
	}

	void video_player::dock_windows(size_t count)
	{
		dock_window_count_ = count;
	}

	const video_player_data& video_player::data() const
	{
		return data_;
	}

	void video_player::set_loop_mode(vt::loop_mode value)
	{
		loop_mode_ = value;
	}

	void video_player::set_playing(bool value)
	{
		is_playing_ = value;
		std::invoke(callbacks.on_set_playing, is_playing_);
	}

	bool video_player::is_visible() const
	{
		return is_visible_;
	}

	loop_mode video_player::loop_mode() const
	{
		return loop_mode_;
	}

	bool video_player::is_playing() const
	{
		return is_playing_;
	}
}
