#include "pch.hpp"
#include "video_timeline.hpp"
// THIS IS A MODIFED VERSION OF THE SEQUENCER WIDGET FROM ImGuizmo
// 
// https://github.com/CedricGuillemet/ImGuizmo
// v 1.89 WIP
//
// The MIT License(MIT)
//
// Copyright(c) 2021 Cedric Guillemet
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#include <algorithm>
#include <cstdlib>
#include <optional>
#include <string_view>
#include <string>

//#include <widgets/tag_manager.hpp>
#include <widgets/tag_menu.hpp>
#include <core/debug.hpp>
#include "insert_segment_popup.hpp"

#include <core/app_context.hpp>
#include <utils/drag_drop.hpp>
#include <editor/set_selected_attribute_command.hpp>

namespace vt::widgets
{
	tag& video_timeline::get_displayed_tag(size_t index)
	{
		return tags_->at(displayed_tags_[current_video_group_id_].at(index));
	}

	void video_timeline::add_displayed_tag(const std::string& name)
	{
		if (current_video_group_id_ == invalid_video_group_id)
		{
			return;
		}

		if (!tags_->contains(name))
		{
			return;
		}

		auto& displayed_tags_vec = displayed_tags_[current_video_group_id_];

		if (std::find(displayed_tags_vec.begin(), displayed_tags_vec.end(), name) != displayed_tags_vec.end())
		{
			return;
		}

		displayed_tags_vec.push_back(name);
		ctx_.is_project_dirty = true;
	}

	void video_timeline::remove_displayed_tag(size_t index)
	{
		auto& displayed_tags_vec = displayed_tags_[current_video_group_id_];

		displayed_tags_vec.erase(displayed_tags_vec.begin() + index);
		ctx_.is_project_dirty = true;
	}

	void video_timeline::sync_tags()
	{
		for (auto& [_, displayed_tags_vec] : displayed_tags_)
		{
			for (auto it = displayed_tags_vec.begin(); it != displayed_tags_vec.end(); ++it)
			{
				if (!tags_->contains(*it))
				{
					it = displayed_tags_vec.erase(it);
					ctx_.is_project_dirty = true;
				}

				if (it == displayed_tags_vec.end())
				{
					break;
				}
			}
		}
	}

	void video_timeline::set_video_group_id(video_group_id_t id)
	{
		current_video_group_id_ = id;
	}

	video_group_id_t video_timeline::video_group_id() const
	{
		return current_video_group_id_;
	}

	void video_timeline::set_tag_storage(vt::tag_storage* tags)
	{
		tags_ = tags;
	}

	tag_storage* video_timeline::tag_storage() const
	{
		return tags_;
	}

	void video_timeline::set_segment_storage(vt::segment_storage* segments)
	{
		segments_ = segments;
	}

	segment_storage* video_timeline::segment_storage() const
	{
		return segments_;
	}

	void video_timeline::set_enabled(bool value)
	{
		enabled_ = value;
	}

	bool video_timeline::is_enabled() const
	{
		return enabled_;
	}

	void video_timeline::set_start_timestamp(timestamp ts)
	{
		time_min_ = ts;
	}

	timestamp video_timeline::start_timestamp() const
	{
		return time_min_;
	}

	void video_timeline::set_end_timestamp(timestamp ts)
	{
		time_max_ = ts;
	}

	timestamp video_timeline::end_timestamp() const
	{
		return time_max_;
	}

	void video_timeline::set_current_timestamp(timestamp ts)
	{
		current_time_ = ts;
	}

	timestamp video_timeline::current_timestamp() const
	{
		return current_time_;
	}

	std::vector<std::string>& video_timeline::displayed_tags()
	{
		return displayed_tags_[current_video_group_id_];
	}

	const std::vector<std::string>& video_timeline::displayed_tags() const
	{
		return displayed_tags_.at(current_video_group_id_);
	}

	std::unordered_map<video_group_id_t, std::vector<std::string>>& video_timeline::displayed_tags_per_group()
	{
		return displayed_tags_;
	}

	const std::unordered_map<video_group_id_t, std::vector<std::string>>& video_timeline::displayed_tags_per_group() const
	{
		return displayed_tags_;
	}


	static bool timeline_add_button(ImDrawList* draw_list, ImVec2 pos, bool enabled)
	{
		//TODO: Use a regular imgui button

		ImGuiIO& io = ImGui::GetIO();
		const float font_scale = io.FontGlobalScale;
		const float button_size = 16.0f * font_scale;
		const ImGuiStyle& style = ImGui::GetStyle();
		ImRect btnRect(pos, ImVec2(pos.x + button_size, pos.y + button_size));
		bool overBtn = btnRect.Contains(io.MousePos) and enabled;
		bool containedClick = overBtn and btnRect.Contains(io.MouseClickedPos[0]);
		bool clickedBtn = containedClick and io.MouseReleased[0];
		//int btnColor = overBtn ? 0xAAEAFFAA : 0x77A3B2AA;
		ImU32 button_color = ImGui::ColorConvertFloat4ToU32(style.Colors[overBtn ? ImGuiCol_Text : ImGuiCol_TextDisabled]);
		if (containedClick and io.MouseDownDuration[0] > 0)
			btnRect.Expand(2.0f);

		float midy = pos.y + button_size / 2 - 0.5f;
		float midx = pos.x + button_size / 2 - 0.5f;
		draw_list->AddRect(btnRect.Min, btnRect.Max, button_color, 4);
		draw_list->AddLine(ImVec2(btnRect.Min.x + 3, midy), ImVec2(btnRect.Max.x - 3, midy), button_color, 2);
		draw_list->AddLine(ImVec2(midx, btnRect.Min.y + 3), ImVec2(midx, btnRect.Max.y - 3), button_color, 2);
		return clickedBtn;
	}

	bool merge_segments_popup(const std::string& id, bool& pressed_button, bool display_dragged_segment_text)
	{
		//TODO: improve layout

		static constexpr ImVec2 button_size = { 55, 30 };

		bool return_value = false;

		auto& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 7);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.WindowPadding * 2);
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		auto flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;

		if (ImGui::BeginPopupModal(id.c_str(), nullptr, flags))
		{
			ctx_.pause_player = true;

			ImGui::Text("Do you want to merge the overlapping segments?");
			if (display_dragged_segment_text) ImGui::TextDisabled("(Pressing \"No\" will move the currently dragged segment back to its original position)");
			ImGui::NewLine();
			auto area_size = ImGui::GetWindowSize();

			ImGui::SetCursorPosX(area_size.x / 2 - button_size.x - 20);
			if (ImGui::Button("Yes", button_size))
			{
				pressed_button = true;
				return_value = true;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			ImGui::SetCursorPosX(area_size.x / 2 + 20);
			if (ImGui::Button("No", button_size) or ImGui::IsKeyPressed(ImGuiKey_Escape))
			{
				pressed_button = false;
				return_value = true;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		ImGui::PopStyleVar(2);

		return return_value;
	}

	//TODO:
	// Improve the context dots_hor
	// Maybe more accurracy than a second.
	// Display time in 10 second intervals instead of 20

	void video_timeline::render(bool& open)
	{
		static std::string window_id = "Timeline";
		ImVec2 default_window_padding = ImGui::GetStyle().WindowPadding;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{});
		if (ImGui::Begin(window_id.c_str(), &open))
		{
			//TODO: check for nulls

			enabled_ = current_video_group_id_ != invalid_video_group_id;

			ImGui::PushID(0);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, default_window_padding);

			ImGuiIO& io = ImGui::GetIO();
			const float font_scale = io.FontGlobalScale;
			ImGuiStyle& style = ImGui::GetStyle();
			int mouse_pos_x = (int)(io.MousePos.x);
			int mouse_pos_y = (int)(io.MousePos.y);
			static float frame_pixel_width = 1.f;
			static constexpr float frame_pixel_width_max = 100.f;
			static float frame_pixel_width_target = frame_pixel_width;
			int legend_width = 200;

			float item_height = 20 * font_scale;

			const int64_t time_min = enabled_ ? time_min_.total_milliseconds.count() : disabled_time_min.total_milliseconds.count();
			const int64_t time_max = enabled_ ? time_max_.total_milliseconds.count() : disabled_time_max.total_milliseconds.count();

			ImGui::BeginGroup();

			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			ImVec2 canvas_pos = ImGui::GetCursorScreenPos();            // ImDrawList API uses screen coordinates!
			ImVec2 canvas_size = ImGui::GetContentRegionAvail();        // Resize canvas to what's available
			int64_t first_frame_used = first_frame_;
			ImVec2 header_size(canvas_size.x, (float)item_height);
			ImVec2 scroll_bar_size(canvas_size.x, style.ScrollbarSize);
			bool has_scroll_bar = true;

			auto& displayed_tags = displayed_tags_[current_video_group_id_];

			float control_height = std::max(std::max(displayed_tags.size(), size_t{ 1 }) * item_height, ImGui::GetWindowSize().y - (has_scroll_bar ? scroll_bar_size.y : 0));
			int64_t frame_count = std::max<int64_t>(time_max - time_min, 1);

			static bool moving_scroll_bar = false;
			static bool moving_time_marker = false;

			// zoom in/out
			const int64_t visibleFrameCount = (int64_t)floorf((canvas_size.x - legend_width) / frame_pixel_width);
			const float barWidthRatio = std::min(visibleFrameCount / (float)frame_count, 1.f);
			const float barWidthInPixels = barWidthRatio * (canvas_size.x - legend_width);

			ImRect region_rect(canvas_pos, canvas_pos + canvas_size);

			frame_pixel_width_target = std::clamp(frame_pixel_width_target, 1.f / frame_pixel_width_max, frame_pixel_width_max);

			frame_pixel_width = ImLerp(frame_pixel_width, frame_pixel_width_target, 0.33f);

			frame_count = time_max - time_min;
			if (visibleFrameCount >= frame_count)
			{
				first_frame_ = time_min;
			}

			{
				/*
				int framesPixelWidth = int(frameCount * framePixelWidth);
				if ((framesPixelWidth + legendWidth) >= canvas_size.x)
				{
					hasScrollBar = true;
				}
				*/
				// test scroll area
				ImGui::InvisibleButton("##TopBar", header_size);
				draw_list->AddRectFilled(canvas_pos, canvas_pos + header_size, 0xFFFF0000, 0);
				ImVec2 childFramePos = ImGui::GetCursorScreenPos();
				ImVec2 childFrameSize(canvas_size.x, canvas_size.y - 8.f - header_size.y - (has_scroll_bar ? scroll_bar_size.y : 0));
				//ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);
				ImGui::BeginChild("##Frame", childFrameSize, ImGuiChildFlags_FrameStyle);

				focused_ = ImGui::IsWindowFocused();
				ImGui::InvisibleButton("##ContentBar", ImVec2(canvas_size.x, float(control_height)), ImGuiButtonFlags_AllowOverlap);
				const ImVec2 contentMin = ImGui::GetItemRectMin();
				const ImVec2 contentMax = ImGui::GetItemRectMax();
				const ImRect contentRect(contentMin, contentMax);
				const float contentHeight = contentMax.y - contentMin.y;

				const ImRect timeline_rect = { {contentMin.x + legend_width, contentMin.y}, contentMax };

				static bool panning_view = false;
				static ImVec2 panning_view_source;
				static int64_t panning_view_frame;

				if (enabled_ and ImGui::IsMouseHoveringRect(timeline_rect.Min, timeline_rect.Max) and io.KeyAlt and io.MouseDown[2])
				{
					if (!panning_view)
					{
						panning_view_source = io.MousePos;
						panning_view = true;
						panning_view_frame = first_frame_;
					}
					first_frame_ = panning_view_frame - int64_t((io.MousePos.x - panning_view_source.x) / frame_pixel_width);
					first_frame_ = std::clamp(first_frame_, time_min, time_max - visibleFrameCount);
				}
				if (panning_view and !io.MouseDown[2])
				{
					panning_view = false;
				}

				if (ctx_.displayed_videos.is_playing() and !moving_scroll_bar and !panning_view)
				{
					if (current_time_.total_milliseconds.count() < first_frame_ or (first_frame_ + visibleFrameCount) <= current_time_.total_milliseconds.count())
					{
						first_frame_ = std::clamp(current_time_.total_milliseconds.count() - visibleFrameCount / 2, int64_t{ 0 }, frame_count - visibleFrameCount);
					}
				}


				auto mouse_pos_to_timestamp = [contentMin, legend_width, first_frame_used](float mouse_pos_x)
				{
					return timestamp{ std::chrono::milliseconds{ static_cast<int64_t>((mouse_pos_x - (contentMin.x + legend_width - first_frame_used * frame_pixel_width)) / frame_pixel_width) } };
				};

				if (enabled_ and ImGui::BeginDragDropTargetCustom(timeline_rect, ImGui::GetID(window_id.c_str())))
				{
					auto payload = utils::drag_drop::get_payload<const char*>("Tag");
					if (payload.data.has_value())
					{
						auto is_delivery = payload.imgui_payload->IsDelivery();
						auto tag_name = std::string(*payload.data);

						if (is_delivery)
						{
							add_displayed_tag(tag_name);

							insert_segment_data insert_data;
							insert_data.show_insert_popup = true;
							insert_data.start = mouse_pos_to_timestamp(io.MousePos.x);
							insert_data.end = *insert_data.start + timestamp(1);
							insert_data.tag = tag_name;

							//TODO: maybe use something more descriptive than an empty string
							(*insert_segment_container)[""] = insert_data;
						}
					}
					ImGui::EndDragDropTarget();
				}

				// full background

				ImU32 bg_color = ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_MenuBarBg]); //0xFF242424
				draw_list->AddRectFilled(canvas_pos, canvas_pos + canvas_size, bg_color, 0);

				// current frame top
				ImRect topRect(ImVec2(canvas_pos.x + legend_width, canvas_pos.y), ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + item_height));

				if (enabled_ and !moving_time_marker and !moving_scroll_bar and !moving_segment.has_value() and current_time_.total_milliseconds.count() >= 0 and topRect.Contains(io.MousePos) and io.MouseClicked[0] and !ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup | ImGuiPopupFlags_AnyPopupId))
				{
					moving_time_marker = true;
				}
				if (moving_time_marker)
				{
					if (frame_count)
					{
						current_time_.total_milliseconds = std::chrono::milliseconds((int)((io.MousePos.x - topRect.Min.x) / frame_pixel_width) + first_frame_used);
						if (current_time_.total_milliseconds.count() < time_min)
						{
							current_time_.total_milliseconds = std::chrono::milliseconds(time_min);
						}
						if (current_time_.total_milliseconds.count() >= time_max)
						{
							current_time_.total_milliseconds = std::chrono::milliseconds(time_max);
						}
					}
					if (!io.MouseDown[0])
					{
						moving_time_marker = false;
					}
				}

				//header
				ImU32 header_color = ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_WindowBg]); //0xFF3D3837
				draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_size.x + canvas_pos.x, canvas_pos.y + item_height), header_color, 0);

				//if (sequence_options & ImSequencer::SEQUENCER_ADD)
				{
					if (timeline_add_button(draw_list, ImVec2(canvas_pos.x + legend_width - item_height, canvas_pos.y + 2), enabled_))
					{
						ImGui::OpenPopup("##AddEntry");
					}

					if (ImGui::BeginPopup("##AddEntry", ImGuiWindowFlags_NoMove))
					{
						//TODO: temporary. the add button should just be disable
						if (current_video_group_id_ == invalid_video_group_id)
						{
							ImGui::CloseCurrentPopup();
						}

						auto selected_tag = tags_->end();

						//TODO: This should already be sorted, not sorted every frame
						std::sort(displayed_tags.begin(), displayed_tags.end());
						bool tags_modifed = false;
						if (tag_menu(*tags_, displayed_tags, tags_modifed))
						{
							ImGui::CloseCurrentPopup();
						}

						if (tags_modifed)
						{
							ctx_.is_project_dirty = true;
						}

						ImGui::EndPopup();
					}
				}

				//header frame number and lines
				int modFrameCount = 10;
				int frameStep = 1;
				while ((modFrameCount * frame_pixel_width) < 150)
				{
					modFrameCount *= 2;
					frameStep *= 2;
				};
				int halfModFrameCount = modFrameCount / 2;

				auto drawLine = [&](int64_t i, float regionHeight) {
					bool baseIndex = ((i % modFrameCount) == 0) || (i == time_max || i == time_min);
					bool halfIndex = (i % halfModFrameCount) == 0;
					float px = canvas_pos.x + i * frame_pixel_width + legend_width - first_frame_used * frame_pixel_width;
					float tiretStart = baseIndex ? 4.0f : (halfIndex ? 10.0f : 14.0f);
					float tiretEnd = baseIndex ? regionHeight : item_height;

					if (px <= (canvas_size.x + canvas_pos.x) and px >= (canvas_pos.x + legend_width))
					{
						draw_list->AddLine(ImVec2((float)px, canvas_pos.y + (float)tiretStart), ImVec2((float)px, canvas_pos.y + (float)tiretEnd - 1), 0xFF606060, 1);

						draw_list->AddLine(ImVec2((float)px, canvas_pos.y + (float)item_height), ImVec2((float)px, canvas_pos.y + (float)regionHeight - 1), 0x30606060, 1);
					}

					if (baseIndex and px > (canvas_pos.x + legend_width))
					{
						timestamp time{ std::chrono::milliseconds(i) };
						std::string time_string = utils::time::time_to_string(time.total_milliseconds.count());
						
						ImU32 text_color = ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_Text]); //0xFFBBBBBB
						draw_list->AddText(ImVec2((float)px + 3.f, canvas_pos.y), text_color, time_string.c_str());
					}

				};

				auto drawLineContent = [&](int64_t i, int /*regionHeight*/) {
					int px = (int)canvas_pos.x + int(i * frame_pixel_width) + legend_width - int(first_frame_used * frame_pixel_width);
					int tiretStart = int(contentMin.y);
					int tiretEnd = int(contentMax.y);

					if (px <= (canvas_size.x + canvas_pos.x) and px >= (canvas_pos.x + legend_width))
					{
						//draw_list->AddLine(ImVec2((float)px, canvas_pos.y + (float)tiretStart), ImVec2((float)px, canvas_pos.y + (float)tiretEnd - 1), 0xFF606060, 1);

						auto line_color4 = style.Colors[ImGuiCol_TextDisabled];
						line_color4.w = 30 / 255.0f;
						ImU32 line_color = ImGui::ColorConvertFloat4ToU32(line_color4); //0x30606060
						draw_list->AddLine(ImVec2(float(px), float(tiretStart)), ImVec2(float(px), float(tiretEnd)), line_color, 1);
					}
				};

				for (int64_t i = time_min; i <= time_max; i += frameStep)
				{
					drawLine(i, item_height);
				}
				drawLine(time_min, item_height);
				drawLine(time_max, item_height);
				/*
				draw_list->AddLine(canvas_pos, ImVec2(canvas_pos.x, canvas_pos.y + controlHeight), 0xFF000000, 1);
				draw_list->AddLine(ImVec2(canvas_pos.x, canvas_pos.y + ItemHeight), ImVec2(canvas_size.x, canvas_pos.y + ItemHeight), 0xFF000000, 1);
				*/
				// clip content

				draw_list->PushClipRect(childFramePos, childFramePos + childFrameSize, true);

				// draw item names in the legend rect on the left
				for (size_t i = 0; i < displayed_tags.size(); i++)
				{
					ImVec2 tpos(contentMin.x + 3, contentMin.y + i * item_height + 2);
					ImU32 text_color = ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_Text]); //0xFFFFFFFF
					draw_list->AddText(tpos, text_color, displayed_tags.at(i).c_str());
				}

				draw_list->PushClipRect(childFramePos + ImVec2(float(legend_width), 0.f), childFramePos + childFrameSize, true);

				// vertical frame lines in content area
				for (int64_t i = time_min; i <= time_max; i += frameStep)
				{
					drawLineContent(i, int(contentHeight));
				}
				drawLineContent(time_min, int(contentHeight));
				drawLineContent(time_max, int(contentHeight));

				// slots
				bool deselect = selected_segment.has_value();
				static std::optional<size_t> mouse_tag_line_index;
				if (!ImGui::IsPopupOpen("##SegmentContextMenu"))
				{
					mouse_tag_line_index.reset();
				}

				if (segments_ != nullptr)
					for (size_t i = 0; i < displayed_tags.size(); i++)
					{
						tag& tag_info = get_displayed_tag(i);
						//TODO: consider using at() instead but then an entry would need to be created somewhere first
						tag_timeline& segments = (*segments_)[tag_info.name];

						for (auto segment_it = segments.begin(); segment_it != segments.end(); ++segment_it)
						{
							if (moving_segment.has_value() and *moving_segment->tag == tag_info and moving_segment->segment_it == segment_it)
							{
								continue;
							}

							auto& tag_segment = *segment_it;

							int64_t start = tag_segment.start.total_milliseconds.count();
							int64_t end = tag_segment.end.total_milliseconds.count();

							uint32_t timestamp_color = tag_info.color & 0x00FFFFFF | 0xFF000000;

							ImVec2 pos = ImVec2(contentMin.x + legend_width - first_frame_used * frame_pixel_width, contentMin.y + item_height * i + 1);
							ImVec2 slot_p1(pos.x + start * frame_pixel_width, pos.y + 2);
							ImVec2 slot_p2(pos.x + end * frame_pixel_width + frame_pixel_width, pos.y + item_height - 2);

							//ImVec2 slotP3(pos.x + end * framePixelWidth + framePixelWidth, pos.y + ItemHeight - 2);
							//TODO: should be somewhere else
							uint32_t selection_color = ImGui::ColorConvertFloat4ToU32({ 1.f, 0xA5 / 255.f, 0.f, 1.f }); //0xFFA500FF
							float selection_thickness = 2.0f;

							bool is_selected = selected_segment.has_value() and selected_segment->segments == &segments and selected_segment->segment_it == segment_it;

							// Drawing
							if (slot_p1.x <= (canvas_size.x + contentMin.x) and slot_p2.x >= (contentMin.x + legend_width))
							{
								if (tag_segment.type() == tag_segment_type::segment)
								{
									//draw_list->AddRectFilled(slotP1, slotP3, slotColorHalf, 2);
									draw_list->AddRectFilled(slot_p1, slot_p2, timestamp_color, 2);
									if (is_selected)
									{
										draw_list->AddRect(slot_p1, slot_p2, selection_color, 2, 0, selection_thickness);
									}
								}
								else if (tag_segment.type() == tag_segment_type::timestamp)
								{
									ImVec2 pos = { (slot_p2.x + slot_p1.x) / 2, slot_p1.y + item_height / 2 - 2 };
									ImVec2 p1 = { pos.x, pos.y - item_height / 2 + 1 };
									ImVec2 p2 = { pos.x + (slot_p2.x - slot_p1.x) / 2, pos.y };
									ImVec2 p3 = { pos.x, pos.y + item_height / 2 - 1 };
									ImVec2 p4 = { pos.x - (slot_p2.x - slot_p1.x) / 2, pos.y };

									draw_list->AddQuadFilled(p1, p2, p3, p4, timestamp_color);
									if (is_selected)
									{
										draw_list->AddQuad(p1, p2, p3, p4, selection_color, selection_thickness);
									}
								}
							}

							/*if (ImRect(slotP1, slotP2).Contains(io.MousePos) and io.MouseDoubleClicked[0])
							{
								state->double_click(i);
							}*/
							// Ensure grabbable handles
							const float max_handle_width = slot_p2.x - slot_p1.x / 3.0f;
							const float min_handle_width = std::min(10.0f, max_handle_width);
							const float handle_width = std::clamp(frame_pixel_width / 2.0f, min_handle_width, max_handle_width);
							ImRect rects[3] = {
								ImRect(slot_p1, ImVec2(slot_p1.x + handle_width, slot_p2.y)),
								ImRect(ImVec2(slot_p2.x - handle_width, slot_p1.y), slot_p2),
								ImRect(slot_p1, slot_p2)
							};

							bool mouse_on_segment = rects[2].Contains(io.MousePos);
							bool is_interactable = rects[2].GetWidth() >= 10.f;

							// Timestamp selection
							if (is_interactable and ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) and (ImGui::IsMouseClicked(ImGuiMouseButton_Left) or ImGui::IsMouseClicked(ImGuiMouseButton_Right)))
							{
								if (mouse_on_segment)
								{
									selected_segment = selected_segment_data
									{
										&tag_info,
										&segments,
										segment_it
									};
									ctx_.registry.execute<set_selected_attribute_command>(nullptr);
									moving_segment.reset();
								}

								deselect &= !mouse_on_segment;
							}

							const unsigned int quadColor[] = { 0xFFFFFFFF, 0xFFFFFFFF, timestamp_color/* + (selected ? 0 : 0x202020)*/ };
							if (is_interactable and ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) and !moving_segment.has_value())// TODOFOCUS and backgroundRect.Contains(io.MousePos))
							{
								for (int j = 2; j >= 0; j--)
								{
									ImRect& rc = rects[j];

									if (!rc.Contains(io.MousePos))
										continue;
									ImGuiMouseCursor cursor = ImGui::GetMouseCursor();
									if ((j == 0 or j == 1) and segment_it->type() != tag_segment_type::timestamp)
									{
										cursor = ImGuiMouseCursor_ResizeEW;
									}
									else if (j == 2)
									{
										cursor = ImGuiMouseCursor_Hand;
									}
									ImGui::SetMouseCursor(cursor);
								}

								for (uint8_t j = 0; j < 3; j++)
								{
									ImRect& rc = rects[j];
									if (!rc.Contains(io.MousePos))
										continue;
									if (!ImRect(childFramePos, childFramePos + childFrameSize).Contains(io.MousePos))
										continue;
									if (ImGui::IsMouseClicked(0) and !moving_scroll_bar and !moving_time_marker and (segment_it->type() != tag_segment_type::timestamp or j == 2))
									{
										moving_segment = moving_segment_data
										{
											&tag_info,
											segment_it,
											static_cast<uint8_t>(j + 1),
											mouse_pos_to_timestamp(io.MousePos.x),
											segment_it->start,
											segment_it->end
										};

										//state->begin_edit(movingEntry);
										break;
									}
								}
							}


							//ImVec2 rp(canvas_pos.x, contentMin.y + ItemHeight * i);
							//ImRect customRect(rp + ImVec2(legendWidth - (firstFrameUsed - time_min - 0.5f) * framePixelWidth, float(0.f)),
							//	rp + ImVec2(legendWidth + (time_max - firstFrameUsed - 0.5f + 2.f) * framePixelWidth, float(ItemHeight)));
							//ImRect clippingRect(rp + ImVec2(float(legendWidth), float(0.f)), rp + ImVec2(canvas_size.x, float(ItemHeight)));
							//
							//
							//
							//compactCustomDraws.push_back({ i, customRect, ImRect(), clippingRect, ImRect() });
						}

						// Moving timestamp drawing
						if (moving_segment.has_value() and *moving_segment->tag == tag_info)
						{
							auto& tag_segment = *moving_segment->segment_it;

							int64_t start = moving_segment->start.total_milliseconds.count();
							int64_t end = moving_segment->end.total_milliseconds.count();
							ImVec2 pos = ImVec2(contentMin.x + legend_width - first_frame_used * frame_pixel_width, contentMin.y + item_height * i + 1);
							ImVec2 slot_p1(pos.x + start * frame_pixel_width, pos.y + 2);
							ImVec2 slot_p2(pos.x + end * frame_pixel_width + frame_pixel_width, pos.y + item_height - 2);

							uint32_t timestamp_color = tag_info.color & 0x00FFFFFF | 0x80000000;
							//TODO: should be somewhere else
							uint32_t selection_color = ImGui::ColorConvertFloat4ToU32({ 1.f, 0xA5 / 255.f, 0.f, 1.f });
							float selection_thickness = 2.0f;

							if (slot_p1.x <= (canvas_size.x + contentMin.x) and slot_p2.x >= (contentMin.x + legend_width))
							{
								if (tag_segment.type() == tag_segment_type::segment)
								{
									//draw_list->AddRectFilled(slotP1, slotP3, slotColorHalf, 2);
									draw_list->AddRectFilled(slot_p1, slot_p2, timestamp_color, 2);
									draw_list->AddRect(slot_p1, slot_p2, selection_color, 2, 0, selection_thickness);
								}
								else if (tag_segment.type() == tag_segment_type::timestamp)
								{
									ImVec2 pos = { (slot_p2.x + slot_p1.x) / 2, slot_p1.y + item_height / 2 - 2 };
									ImVec2 p1 = { pos.x, pos.y - item_height / 2 + 1 };
									ImVec2 p2 = { pos.x + (slot_p2.x - slot_p1.x) / 2, pos.y };
									ImVec2 p3 = { pos.x, pos.y + item_height / 2 - 1 };
									ImVec2 p4 = { pos.x - (slot_p2.x - slot_p1.x) / 2, pos.y };

									draw_list->AddQuadFilled(p1, p2, p3, p4, timestamp_color);
									draw_list->AddQuad(p1, p2, p3, p4, selection_color, selection_thickness);
								}
							}
						}

						ImVec2 rp = { canvas_pos.x, contentMin.y + item_height * i };
						ImRect tag_line_rect = { rp + ImVec2(float(legend_width), float(0.f)), rp + ImVec2(canvas_size.x, float(item_height)) };

						if (!ImGui::IsPopupOpen("##SegmentContextMenu") and tag_line_rect.Contains(io.MousePos))
						{
							mouse_tag_line_index = i;
						}
					}

				//TODO: improve
				// Segment context menu
				if (enabled_ and !displayed_tags.empty())
				{
					static timestamp inserted_segment_start{};
					static timestamp inserted_segment_end{};
					static tag* tag_info{};
					static int selected_tag{};

					tag_timeline* segments{};
					if (mouse_tag_line_index.has_value())
					{
						tag_info = &get_displayed_tag(*mouse_tag_line_index);
						segments = &segments_->at(tag_info->name);
					}

					bool ready_to_insert = false;

					static timestamp mouse_timestamp;

					bool mouse_on_timeline = ImGui::IsMouseHoveringRect(timeline_rect.Min, timeline_rect.Max);
					if (ImGui::BeginPopupContextItem("##SegmentContextMenu"))
					{
						if (ImGui::IsWindowAppearing())
						{
							if (!mouse_on_timeline)
							{
								ImGui::CloseCurrentPopup();
							}

							mouse_timestamp = mouse_pos_to_timestamp(io.MousePos.x);
						}

						auto selected_timepoint = mouse_timestamp;

						std::optional<tag_timeline::iterator> segment_it;
						if (segments != nullptr)
						{
							segment_it = segments->find(selected_timepoint);
							if (*segment_it == segments->end())
							{
								segment_it.reset();
							}
						}

						if (!segment_it.has_value())
						{
							if (ImGui::MenuItem("Add timestamp"))
							{
								inserted_segment_start = mouse_timestamp;
								inserted_segment_end = inserted_segment_start;
								ready_to_insert = true;
							}
							if (ImGui::MenuItem("Add segment"))
							{
								inserted_segment_start = mouse_timestamp;
								inserted_segment_end = inserted_segment_start + timestamp(tag_segment::default_segment_size);
								ready_to_insert = true;
							}
							ImGui::SeparatorText("Marker");
							//TODO: maybe could use the moving timestamp
							if (ImGui::MenuItem("Add timestamp at marker"))
							{
								inserted_segment_start = current_time_;
								inserted_segment_end = inserted_segment_start;
								ready_to_insert = true;
							}
							if (ImGui::MenuItem("Add segment at marker"))
							{
								//TODO: should draw a line or something so you know where you clicked
								inserted_segment_start = current_time_;
								inserted_segment_end = current_time_ + timestamp(tag_segment::default_segment_size);
								ready_to_insert = true;
							}
							if (ImGui::MenuItem("Start segment at marker"))
							{
								//TODO: should draw a line or something so you know where you clicked

								//***********
								//TODO: THIS SHOULD INSERT INTO insert_segment_container BUT WITH ready = false
								// ***********
								inserted_segment_start = current_time_;
							}
							//TODO: probably should only be displayed after start was pressed
							if (ImGui::MenuItem("End segment at marker"))
							{
								inserted_segment_end = current_time_;
								ready_to_insert = true;
							}
						}
						else
						{
							if (ImGui::MenuItem((*segment_it)->type() == tag_segment_type::timestamp ? "Delete timestamp" : "Delete segment"))
							{
								segments->erase(*segment_it);
								if (selected_segment.has_value() and selected_segment->segments == segments and selected_segment->segment_it == *segment_it)
								{
									ctx_.registry.execute<set_selected_attribute_command>(nullptr);
									selected_segment.reset();
									moving_segment.reset();
								}
							}
						}
						ImGui::EndPopup();
					}

					if (ready_to_insert)
					{
						if (segments != nullptr)
						{
							auto it = std::find(displayed_tags.begin(), displayed_tags.end(), tag_info->name);
							selected_tag = (it != displayed_tags.end()) ? static_cast<int>(it - displayed_tags.begin()) : 0;
						}

						insert_segment_data insert_data;
						insert_data.show_insert_popup = true;
						insert_data.start = inserted_segment_start;
						insert_data.end = inserted_segment_end;
						insert_data.tag = displayed_tags[selected_tag];

						//TODO: maybe use something more descriptive than an empty string
						(*insert_segment_container)[""] = insert_data;
					}
				}

				if (ImGui::IsMouseHoveringRect(contentMin, contentMax) and ImGui::IsMouseClicked(ImGuiMouseButton_Left) and deselect)
				{
					ctx_.registry.execute<set_selected_attribute_command>(nullptr);
					selected_segment.reset();
					moving_segment.reset();
				}

				if (moving_segment.has_value())
				{
					if (moving_segment->grab_part == 1 or moving_segment->grab_part == 2)
					{
						ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
					}
				}

				// moving
				if (enabled_ and /*region_rect.Contains(io.MousePos) and*/ moving_segment.has_value() and !ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup))
				{
					ImGui::SetNextFrameWantCaptureMouse(true);
					auto mouse_timestamp = mouse_pos_to_timestamp(io.MousePos.x);
					auto move_delta = mouse_timestamp - moving_segment->grab_position;

					//auto diff_sec = mouse_pos_x - segment_moving_data->position.count();
					//auto diffFrame = std::chrono::seconds{ int64_t(diff_sec / framePixelWidth) };
					if (std::abs(move_delta.total_milliseconds.count()) > 0 and (time_min_ <= mouse_timestamp and mouse_timestamp <= time_max_))
					{
						/*if (selected_entry)
							*selected_entry = movingEntry;*/


						if (moving_segment->grab_part & 1)
							moving_segment->start += move_delta;
						if (moving_segment->grab_part & 2)
							moving_segment->end += move_delta;
						if (moving_segment->start < timestamp::zero())
						{
							if (moving_segment->grab_part & 2)
								moving_segment->end -= moving_segment->start;
							moving_segment->start = timestamp::zero();
						}
						if (moving_segment->grab_part & 1 and moving_segment->start > moving_segment->end)
							moving_segment->start = moving_segment->end;
						if (moving_segment->grab_part & 2 and moving_segment->end < moving_segment->start)
							moving_segment->end = moving_segment->start;

						auto segment_size = std::abs((moving_segment->end - moving_segment->start).total_milliseconds.count());
						if (segment_size < tag_segment::min_segment_size.count() and moving_segment->segment_it->type() != tag_segment_type::timestamp)
						{
							if (moving_segment->grab_part & 1)
							{
								moving_segment->start -= timestamp(tag_segment::min_segment_size - static_cast<std::chrono::milliseconds>(segment_size));
							}
							else if (moving_segment->grab_part & 2)
							{
								moving_segment->end += timestamp(tag_segment::min_segment_size - static_cast<std::chrono::milliseconds>(segment_size));
							}
						}
						moving_segment->grab_position = mouse_timestamp;
					}
					if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
					{
						auto& segments = segments_->at(moving_segment->tag->name);

						//No idea what this was supposed to be used for
						//bool was_selected = selected_timestamp.has_value() and selected_timestamp->timestamp_timeline == &timeline and selected_timestamp->timestamp == segment_moving_data->segment;

						if (selected_segment.has_value())
						{
							auto overlapping = segments.find_range(moving_segment->start, moving_segment->end);

							bool insert_now = true;
							for (auto it = overlapping.begin(); it != overlapping.end(); ++it)
							{
								if (it != selected_segment->segment_it)
								{
									insert_now = false;
								}
							}

							if (insert_now)
							{
								if (selected_segment->segment_it->start != moving_segment->start or selected_segment->segment_it->end != moving_segment->end)
								{
									selected_segment->segment_it = segments.replace
									(
										selected_segment->segment_it,
										moving_segment->start,
										moving_segment->end
									).first;

									ctx_.is_project_dirty = true;
								}
								moving_segment.reset();
							}
							else
							{
								ImGui::OpenPopup("##MergeSegments");
							}
						}
					}
				}
				draw_list->PopClipRect();
				draw_list->PopClipRect();

				bool pressed_yes{};
				if (merge_segments_popup("##MergeSegments", pressed_yes, true))
				{
					if (pressed_yes and moving_segment.has_value())
					{
						auto& segments = segments_->at(moving_segment->tag->name);
						selected_segment->segment_it = segments.replace
						(
							selected_segment->segment_it,
							timestamp{ moving_segment->start },
							timestamp{ moving_segment->end }
						).first;

						ctx_.is_project_dirty = true;
					}
					moving_segment.reset();
				}

				// cursor
				if (current_time_.total_milliseconds.count() >= first_frame_ and current_time_.total_milliseconds.count() <= time_max)
				{
					static constexpr float cursorWidth = 4.f;
					static constexpr float triangle_span = cursorWidth * 2;
					float cursorOffset = contentMin.x + legend_width + (current_time_.total_milliseconds.count() - first_frame_used) * frame_pixel_width + frame_pixel_width / 2 - cursorWidth * 0.5f;
					ImU32 cursor_color = enabled_ ? 0xFF3E36FF : 0xFF3E3E3E; //0xA02A2AFF
					draw_list->AddLine(ImVec2(cursorOffset, canvas_pos.y), ImVec2(cursorOffset, contentMax.y), cursor_color, cursorWidth);
					draw_list->AddTriangleFilled(
						ImVec2(cursorOffset - triangle_span, canvas_pos.y),
						ImVec2(cursorOffset, canvas_pos.y + item_height * 0.5f),
						ImVec2(cursorOffset + triangle_span, canvas_pos.y), cursor_color
					);
				}

				ImGui::EndChildFrame();
				//ImGui::PopStyleColor();
				if (enabled_ and has_scroll_bar)
				{
					ImU32 scroll_bg_color = ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_ScrollbarBg]);
					ImU32 scroll_bg_alt_color = ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_MenuBarBg]);
					ImU32 scroll_color = ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_ScrollbarGrab]);
					ImU32 scroll_hover_color = ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_ScrollbarGrabHovered]);
					ImU32 scroll_active_color = ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_ScrollbarGrabActive]);

					ImGui::InvisibleButton("scrollBar", scroll_bar_size);
					ImVec2 scrollBarMin = ImGui::GetItemRectMin();
					ImVec2 scrollBarMax = ImGui::GetItemRectMax();

					// ratio = number of frames visible in control / number to total frames

					float startFrameOffset = ((float)(first_frame_used - time_min) / (float)frame_count) * (canvas_size.x - legend_width);
					ImVec2 scrollBarA(scrollBarMin.x + legend_width, scrollBarMin.y - 2);
					ImVec2 scrollBarB(scrollBarMin.x + canvas_size.x, scrollBarMax.y - 1);
					draw_list->AddRectFilled(scrollBarA, scrollBarB, scroll_bg_alt_color, 0);

					ImRect scrollBarRect(scrollBarA, scrollBarB);
					bool inScrollBar = scrollBarRect.Contains(io.MousePos);

					draw_list->AddRectFilled(scrollBarA, scrollBarB, scroll_bg_color, style.ScrollbarRounding);


					ImVec2 scrollBarC(scrollBarMin.x + legend_width + startFrameOffset, scrollBarMin.y);
					ImVec2 scrollBarD(scrollBarMin.x + legend_width + barWidthInPixels + startFrameOffset, scrollBarMax.y - 2);
					draw_list->AddRectFilled(scrollBarC, scrollBarD, (inScrollBar || moving_scroll_bar) ? scroll_active_color : scroll_color, style.ScrollbarRounding);

					ImRect barHandleLeft(scrollBarC, ImVec2(scrollBarC.x + style.ScrollbarSize, scrollBarD.y));
					ImRect barHandleRight(ImVec2(scrollBarD.x - style.ScrollbarSize, scrollBarC.y), scrollBarD);

					bool onLeft = barHandleLeft.Contains(io.MousePos);
					bool onRight = barHandleRight.Contains(io.MousePos);

					static bool sizingRBar = false;
					static bool sizingLBar = false;


					draw_list->AddRectFilled(barHandleLeft.Min, barHandleLeft.Max, (onLeft || sizingLBar) ? scroll_hover_color : scroll_active_color, style.ScrollbarRounding);
					draw_list->AddRectFilled(barHandleRight.Min, barHandleRight.Max, (onRight || sizingRBar) ? scroll_hover_color : scroll_active_color, style.ScrollbarRounding);

					ImRect scrollBarThumb(scrollBarC, scrollBarD);
					static const float MinBarWidth = 44.f;
					if (sizingRBar)
					{
						if (!io.MouseDown[0])
						{
							sizingRBar = false;
						}
						else
						{
							float barNewWidth = std::max(barWidthInPixels + io.MouseDelta.x, MinBarWidth);
							float barRatio = barNewWidth / barWidthInPixels;
							frame_pixel_width_target = frame_pixel_width = frame_pixel_width / barRatio;
							int64_t newVisibleFrameCount = int64_t((canvas_size.x - legend_width) / frame_pixel_width_target);
							int64_t lastFrame = first_frame_ + newVisibleFrameCount;
							if (lastFrame > time_max)
							{
								frame_pixel_width_target = frame_pixel_width = (canvas_size.x - legend_width) / float(time_max - first_frame_);
							}
						}
					}
					else if (sizingLBar)
					{
						if (!io.MouseDown[0])
						{
							sizingLBar = false;
						}
						else
						{
							if (fabsf(io.MouseDelta.x) > FLT_EPSILON)
							{
								float barNewWidth = std::max(barWidthInPixels - io.MouseDelta.x, MinBarWidth);
								float barRatio = barNewWidth / barWidthInPixels;
								float previousFramePixelWidthTarget = frame_pixel_width_target;
								frame_pixel_width_target = frame_pixel_width = frame_pixel_width / barRatio;
								int64_t newVisibleFrameCount = int64_t(visibleFrameCount / barRatio);
								int64_t newFirstFrame = first_frame_ + newVisibleFrameCount - visibleFrameCount;
								newFirstFrame = std::clamp(newFirstFrame, time_min, std::max(time_max - visibleFrameCount, time_min));
								if (newFirstFrame == first_frame_)
								{
									frame_pixel_width = frame_pixel_width_target = previousFramePixelWidthTarget;
								}
								else
								{
									first_frame_ = newFirstFrame;
								}
							}
						}
					}
					else
					{
						if (moving_scroll_bar)
						{
							if (!io.MouseDown[0])
							{
								moving_scroll_bar = false;
							}
							else
							{
								float framesPerPixelInBar = barWidthInPixels / (float)visibleFrameCount;
								first_frame_ = int64_t((io.MousePos.x - panning_view_source.x) / framesPerPixelInBar) - panning_view_frame;
								first_frame_ = std::clamp(first_frame_, time_min, std::max(time_max - visibleFrameCount, time_min));
							}
						}
						else
						{
							if (scrollBarThumb.Contains(io.MousePos) and ImGui::IsMouseClicked(0) and !moving_segment.has_value())
							{
								moving_scroll_bar = true;
								panning_view_source = io.MousePos;
								panning_view_frame = -first_frame_;
							}
							if (!sizingRBar and onRight and ImGui::IsMouseClicked(0))
								sizingRBar = true;
							if (!sizingLBar and onLeft and ImGui::IsMouseClicked(0))
								sizingLBar = true;

						}
					}
				}
			}

			ImGui::EndGroup();

			ImGui::PopStyleVar();
			ImGui::PopID();
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}
}
