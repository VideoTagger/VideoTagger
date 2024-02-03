#include "video_timeline.hpp"

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

#include <iostream>

//#include <widgets/tag_manager.hpp>
#include <widgets/tag_menu.hpp>

namespace vt::widgets
{
	tag& timeline_state::get(int index)
	{
		return tags->at(displayed_tags.at(index));
	}

	void timeline_state::add(const std::string& name)
	{
		if (!tags->contains(name))
		{
			return;
		}

		if (std::find(displayed_tags.begin(), displayed_tags.end(), name) != displayed_tags.end())
		{
			return;
		}

		displayed_tags.push_back(name);
	}

	void timeline_state::del(int index)
	{
		displayed_tags.erase(displayed_tags.begin() + index);
	}

	void timeline_state::sync_tags()
	{
		for (auto it = displayed_tags.begin(); it != displayed_tags.end(); ++it)
		{
			if (!tags->contains(*it))
			{
				it = displayed_tags.erase(it);
			}

			if (it == displayed_tags.end())
			{
				break;
			}
		}
	}

	static bool SequencerAddDelButton(ImDrawList* draw_list, ImVec2 pos, bool add = true)
	{
		ImGuiIO& io = ImGui::GetIO();
		const float font_scale = io.FontGlobalScale;
		const float button_size = 16.0f * font_scale;
		const ImGuiStyle& style = ImGui::GetStyle();
		ImRect btnRect(pos, ImVec2(pos.x + button_size, pos.y + button_size));
		bool overBtn = btnRect.Contains(io.MousePos);
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
		if (add)
			draw_list->AddLine(ImVec2(midx, btnRect.Min.y + 3), ImVec2(midx, btnRect.Max.y - 3), button_color, 2);
		return clickedBtn;
	}

	struct moving_tag_data
	{
		tag* tag{};
		tag_timeline::iterator segment{};
		uint8_t grab_part{};
		timestamp grab_position{};
		timestamp left_position{};
		timestamp right_position{};
	};

	//TODO:
	// Improve the context dots_hor
	// Button to remove displayed tag.
	// Maybe more accurracy than a second.
	// Display time in 10 second gaps instead of 20
	// Draggable timepoint segment
	// Merge tags popup

	bool video_timeline(timeline_state& state, timestamp& current_time, std::optional<selected_timestamp_data>& selected_timestamp, bool& dirty_flag)
	{
		bool return_value = false;
		ImGuiIO& io = ImGui::GetIO();
		const float font_scale = io.FontGlobalScale;
		ImGuiStyle& style = ImGui::GetStyle();
		int mouse_pos_x = (int)(io.MousePos.x);
		int mouse_pos_y = (int)(io.MousePos.y);
		static float framePixelWidth = 10.f;
		static float framePixelWidthTarget = 10.f;
		int legendWidth = 200;

		//static int movingEntry = -1;
		static std::optional<moving_tag_data> segment_moving_data;
		int delEntry = -1;
		float ItemHeight = 20 * font_scale;

		const int64_t time_min = state.time_min.seconds_total.count();
		const int64_t time_max = state.time_max.seconds_total.count();

		bool popupOpened = false;

		//TODO: when there's no tags nothing would display. prevent this
		//if (!sequenceCount)
		//	return false;
		ImGui::BeginGroup();

		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 canvas_pos = ImGui::GetCursorScreenPos();            // ImDrawList API uses screen coordinates!
		ImVec2 canvas_size = ImGui::GetContentRegionAvail();        // Resize canvas to what's available
		int64_t firstFrameUsed = state.first_frame;
		ImVec2 headerSize(canvas_size.x, (float)ItemHeight);
		ImVec2 scrollBarSize(canvas_size.x, 14.f);
		bool hasScrollBar(true);

		//TODO: temporary solution to nothing displaying
		//int controlHeight = std::max(sequenceCount, 1) * ItemHeight;
		float controlHeight = std::max(std::max(state.displayed_tags.size(), size_t{1}) * ItemHeight, ImGui::GetWindowSize().y - (hasScrollBar ? scrollBarSize.y : 0));
		int64_t frameCount = std::max(time_max - time_min, 1ll);

		static bool moving_scroll_bar = false;
		static bool moving_time_marker = false;
		struct CustomDraw
		{
			int index;
			ImRect customRect;
			ImRect legendRect;
			ImRect clippingRect;
			ImRect legendClippingRect;
		};
		//ImVector<CustomDraw> customDraws;
		//ImVector<CustomDraw> compactCustomDraws;
		// zoom in/out
		const int64_t visibleFrameCount = (int64_t)floorf((canvas_size.x - legendWidth) / framePixelWidth);
		const float barWidthRatio = std::min(visibleFrameCount / (float)frameCount, 1.f);
		const float barWidthInPixels = barWidthRatio * (canvas_size.x - legendWidth);

		ImRect regionRect(canvas_pos, canvas_pos + canvas_size);

		static bool panningView = false;
		static ImVec2 panningViewSource;
		static int64_t panningViewFrame;
		if (ImGui::IsWindowFocused() and io.KeyAlt and io.MouseDown[2])
		{
			if (!panningView)
			{
				panningViewSource = io.MousePos;
				panningView = true;
				panningViewFrame = state.first_frame;
			}
			state.first_frame = panningViewFrame - int64_t((io.MousePos.x - panningViewSource.x) / framePixelWidth);
			state.first_frame = std::clamp(state.first_frame, time_min, time_max - visibleFrameCount);
		}
		if (panningView and !io.MouseDown[2])
		{
			panningView = false;
		}
		framePixelWidthTarget = std::clamp(framePixelWidthTarget, 0.1f, 50.f);

		framePixelWidth = ImLerp(framePixelWidth, framePixelWidthTarget, 0.33f);

		frameCount = time_max - time_min;
		if (visibleFrameCount >= frameCount)
			state.first_frame = time_min;


		// --
		/*if (expanded and !*expanded)
		{
			ImGui::InvisibleButton("canvas", ImVec2(canvas_size.x - canvas_pos.x, (float)ItemHeight));
			draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_size.x + canvas_pos.x, canvas_pos.y + ItemHeight), 0xFF3D3837, 0);
			char tmps[512];
			ImFormatString(tmps, IM_ARRAYSIZE(tmps), state->get_collapse_fmt(), frameCount, sequenceCount);
			draw_list->AddText(ImVec2(canvas_pos.x + 26, canvas_pos.y + 2), 0xFFFFFFFF, tmps);
		}
		else*/
		{
			/*
			int framesPixelWidth = int(frameCount * framePixelWidth);
			if ((framesPixelWidth + legendWidth) >= canvas_size.x)
			{
				hasScrollBar = true;
			}
			*/
			// test scroll area
			ImGui::InvisibleButton("##TopBar", headerSize);
			draw_list->AddRectFilled(canvas_pos, canvas_pos + headerSize, 0xFFFF0000, 0);
			ImVec2 childFramePos = ImGui::GetCursorScreenPos();
			ImVec2 childFrameSize(canvas_size.x, canvas_size.y - 8.f - headerSize.y - (hasScrollBar ? scrollBarSize.y : 0));
			ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);
			ImGui::BeginChildFrame(889, childFrameSize);
			state.focused = ImGui::IsWindowFocused();
			ImGui::InvisibleButton("##ContentBar", ImVec2(canvas_size.x, float(controlHeight)), ImGuiButtonFlags_AllowOverlap);
			const ImVec2 contentMin = ImGui::GetItemRectMin();
			const ImVec2 contentMax = ImGui::GetItemRectMax();
			const ImRect contentRect(contentMin, contentMax);
			const float contentHeight = contentMax.y - contentMin.y;


			auto mouse_pos_to_timestamp = [contentMin, legendWidth, firstFrameUsed](float mouse_pos_x)
			{
				return timestamp{ std::chrono::seconds{ static_cast<int64_t>((mouse_pos_x - (contentMin.x + legendWidth - firstFrameUsed * framePixelWidth)) / framePixelWidth) } };
			};

			// full background
			
			ImU32 bg_color = ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_MenuBarBg]); //0xFF242424
			draw_list->AddRectFilled(canvas_pos, canvas_pos + canvas_size, bg_color, 0);

			// current frame top
			ImRect topRect(ImVec2(canvas_pos.x + legendWidth, canvas_pos.y), ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + ItemHeight));

			if (!moving_time_marker and !moving_scroll_bar and !segment_moving_data.has_value() and current_time.seconds_total.count() >= 0 and topRect.Contains(io.MousePos) and io.MouseDown[0] and !ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup | ImGuiPopupFlags_AnyPopupId))
			{
				moving_time_marker = true;
			}
			if (moving_time_marker)
			{
				if (frameCount)
				{
					current_time.seconds_total = std::chrono::seconds((int)((io.MousePos.x - topRect.Min.x) / framePixelWidth) + firstFrameUsed);
					if (current_time.seconds_total.count() < time_min)
						current_time.seconds_total = std::chrono::seconds(time_min);
					if (current_time.seconds_total.count() >= time_max)
						current_time.seconds_total = std::chrono::seconds(time_max);
				}
				if (!io.MouseDown[0])
					moving_time_marker = false;
			}

			//header
			ImU32 header_color = ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_WindowBg]); //0xFF3D3837
			draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_size.x + canvas_pos.x, canvas_pos.y + ItemHeight), header_color, 0);
			
			//if (sequence_options & ImSequencer::SEQUENCER_ADD)
			{
				if (SequencerAddDelButton(draw_list, ImVec2(canvas_pos.x + legendWidth - ItemHeight, canvas_pos.y + 2), true))
					ImGui::OpenPopup("##AddEntry");

				if (ImGui::BeginPopup("##AddEntry", ImGuiWindowFlags_NoMove))
				{
					popupOpened = true;

					auto selected_tag = state.tags->end();

					if (tag_menu(*state.tags, state.displayed_tags))
					{

					}
					/*
					if (tag_manager(*state.tags, selected_tag, tag_manager_flags::no_remove))
					{
						state.add(selected_tag->name);
						ImGui::CloseCurrentPopup();
						popupOpened = false;
					}
					*/

					ImGui::EndPopup();
				}
			}

			//header frame number and lines
			int modFrameCount = 10;
			int frameStep = 1;
			while ((modFrameCount * framePixelWidth) < 150)
			{
				modFrameCount *= 2;
				frameStep *= 2;
			};
			int halfModFrameCount = modFrameCount / 2;

			auto drawLine = [&](int64_t i, float regionHeight) {
				bool baseIndex = ((i % modFrameCount) == 0) || (i == time_max || i == time_min);
				bool halfIndex = (i % halfModFrameCount) == 0;
				float px = canvas_pos.x + i * framePixelWidth + legendWidth - firstFrameUsed * framePixelWidth;
				float tiretStart = baseIndex ? 4.0f : (halfIndex ? 10.0f : 14.0f);
				float tiretEnd = baseIndex ? regionHeight : ItemHeight;

				if (px <= (canvas_size.x + canvas_pos.x) and px >= (canvas_pos.x + legendWidth))
				{
					draw_list->AddLine(ImVec2((float)px, canvas_pos.y + (float)tiretStart), ImVec2((float)px, canvas_pos.y + (float)tiretEnd - 1), 0xFF606060, 1);

					draw_list->AddLine(ImVec2((float)px, canvas_pos.y + (float)ItemHeight), ImVec2((float)px, canvas_pos.y + (float)regionHeight - 1), 0x30606060, 1);
				}

				if (baseIndex and px > (canvas_pos.x + legendWidth))
				{
					timestamp time{ std::chrono::seconds(i) };
					char tmps[512];
					ImFormatString(tmps, IM_ARRAYSIZE(tmps), "%02d:%02d:%02d", time.hours(), time.minutes(), time.seconds());
					ImU32 text_color = ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_Text]); //0xFFBBBBBB
					draw_list->AddText(ImVec2((float)px + 3.f, canvas_pos.y), text_color, tmps);
				}

			};

			auto drawLineContent = [&](int64_t i, int /*regionHeight*/) {
				int px = (int)canvas_pos.x + int(i * framePixelWidth) + legendWidth - int(firstFrameUsed * framePixelWidth);
				int tiretStart = int(contentMin.y);
				int tiretEnd = int(contentMax.y);

				if (px <= (canvas_size.x + canvas_pos.x) and px >= (canvas_pos.x + legendWidth))
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
				drawLine(i, ItemHeight);
			}
			drawLine(time_min, ItemHeight);
			drawLine(time_max, ItemHeight);
			/*
			draw_list->AddLine(canvas_pos, ImVec2(canvas_pos.x, canvas_pos.y + controlHeight), 0xFF000000, 1);
			draw_list->AddLine(ImVec2(canvas_pos.x, canvas_pos.y + ItemHeight), ImVec2(canvas_size.x, canvas_pos.y + ItemHeight), 0xFF000000, 1);
			*/
			// clip content

			draw_list->PushClipRect(childFramePos, childFramePos + childFrameSize, true);

			// draw item names in the legend rect on the left
			for (size_t i = 0; i < state.displayed_tags.size(); i++)
			{
				ImVec2 tpos(contentMin.x + 3, contentMin.y + i * ItemHeight + 2);
				ImU32 text_color = ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_Text]); //0xFFFFFFFF
				draw_list->AddText(tpos, text_color, state.displayed_tags.at(i).c_str());

				/*if (sequence_options & ImSequencer::SEQUENCER_DEL)
				{
					if (SequencerAddDelButton(draw_list, ImVec2(contentMin.x + legendWidth - ItemHeight + 2 - 10, tpos.y + 2), false))
						delEntry = i;

					if (SequencerAddDelButton(draw_list, ImVec2(contentMin.x + legendWidth - ItemHeight - ItemHeight + 2 - 10, tpos.y + 2), true))
						dupEntry = i;
				}*/
				//customHeight += state->get_custom_height(i);
			}

			// slots background
			//auto slots_color4 = style.Colors[ImGuiCol_WindowBg];
			//slots_color4.w *= 0.5f;
			//ImU32 slots_color = ImGui::ColorConvertFloat4ToU32(slots_color4); //0xFF413D3D
			//
			//for (int i = 0; i < sequenceCount; i++)
			//{
			//	//TODO: Change this
			//	unsigned int col = (i & 1) ? 0xFF3A3636 : 0xFF413D3D;
			//	//ImU32 col = slots_color;
			//
			//	ImVec2 pos = ImVec2(contentMin.x + legendWidth, contentMin.y + ItemHeight * i + 1);
			//	ImVec2 sz = ImVec2(canvas_size.x + canvas_pos.x, pos.y + ItemHeight - 1);
			//	if (!popupOpened and mouse_pos_y >= pos.y and mouse_pos_y < pos.y + (ItemHeight) and !segment_moving_data.has_value() and mouse_pos_x>contentMin.x and mouse_pos_x < contentMin.x + canvas_size.x)
			//	{
			//		col += 0x80201008;
			//		pos.x -= legendWidth;
			//	}
			//	//draw_list->AddRectFilled(pos, sz, col, 0);
			//}

			draw_list->PushClipRect(childFramePos + ImVec2(float(legendWidth), 0.f), childFramePos + childFrameSize, true);

			// vertical frame lines in content area
			for (int64_t i = time_min; i <= time_max; i += frameStep)
			{
				drawLineContent(i, int(contentHeight));
			}
			drawLineContent(time_min, int(contentHeight));
			drawLineContent(time_max, int(contentHeight));

			// selection
			/*bool selected = selected_entry and (*selected_entry >= 0);
			if (selected)
			{
				//TODO: Change color
				draw_list->AddRectFilled(ImVec2(contentMin.x, contentMin.y + ItemHeight * *selected_entry),
					ImVec2(contentMin.x + canvas_size.x, contentMin.y + ItemHeight * (*selected_entry + 1)), 0x801080FF, 1.f);
			}*/

			// slots
			bool deselect = selected_timestamp.has_value();
			for (size_t i = 0; i < state.displayed_tags.size(); i++)
			{
				tag& tag_info = state.get(i);

				for (auto timestamp_it = tag_info.timeline.begin(); timestamp_it != tag_info.timeline.end(); ++timestamp_it)
				{
					auto& tag_timestamp = *timestamp_it;

					int64_t start = tag_timestamp.start.seconds_total.count();
					int64_t end = tag_timestamp.end.seconds_total.count();
					if (segment_moving_data.has_value() and segment_moving_data->tag == &tag_info and segment_moving_data->segment == timestamp_it)
					{
						start = segment_moving_data->left_position.seconds_total.count();
						end = segment_moving_data->right_position.seconds_total.count();
					}


					uint32_t color = tag_info.color;

					ImVec2 pos = ImVec2(contentMin.x + legendWidth - firstFrameUsed * framePixelWidth, contentMin.y + ItemHeight * i + 1);
					ImVec2 slot_p1(pos.x + start * framePixelWidth, pos.y + 2);
					ImVec2 slot_p2(pos.x + end * framePixelWidth + framePixelWidth, pos.y + ItemHeight - 2);
					//ImVec2 slotP3(pos.x + end * framePixelWidth + framePixelWidth, pos.y + ItemHeight - 2);
					uint32_t slot_color = color | 0xFF000000;
					uint32_t slot_color_half = (color & 0xFFFFFF) | 0x40000000;
					uint32_t selection_color = ImGui::ColorConvertFloat4ToU32({ 1.f, 0xA5 / 255.f, 0.f, 1.f }); //0xFFA500FF
					float selection_thickness = 2.0f;

					bool is_selected = selected_timestamp.has_value() and selected_timestamp->timestamp_timeline == &tag_info.timeline and selected_timestamp->timestamp == timestamp_it;

					if (slot_p1.x <= (canvas_size.x + contentMin.x) and slot_p2.x >= (contentMin.x + legendWidth))
					{
						if (tag_timestamp.type() == tag_timestamp_type::segment)
						{
							//draw_list->AddRectFilled(slotP1, slotP3, slotColorHalf, 2);
							draw_list->AddRectFilled(slot_p1, slot_p2, slot_color, 2);
							if (is_selected)
							{
								draw_list->AddRect(slot_p1, slot_p2, selection_color, 2, 0, selection_thickness);
							}
						}
						else if (tag_timestamp.type() == tag_timestamp_type::point)
						{
							ImVec2 pos = { (slot_p2.x + slot_p1.x) / 2, slot_p1.y + ItemHeight / 2 - 2 };
							ImVec2 p1 = { pos.x, pos.y - ItemHeight / 2 + 1 };
							ImVec2 p2 = { pos.x + (slot_p2.x - slot_p1.x) / 2, pos.y };
							ImVec2 p3 = { pos.x, pos.y + ItemHeight / 2 - 1 };
							ImVec2 p4 = { pos.x - (slot_p2.x - slot_p1.x) / 2, pos.y };

							draw_list->AddQuadFilled(p1, p2, p3, p4, slot_color);
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
					const float handle_width = std::clamp(framePixelWidth / 2.0f, min_handle_width, max_handle_width);
					ImRect rects[3] = {
						ImRect(slot_p1, ImVec2(slot_p1.x + handle_width, slot_p2.y)),
						ImRect(ImVec2(slot_p2.x - handle_width, slot_p1.y), slot_p2),
						ImRect(slot_p1, slot_p2)
					};
					
					bool mouse_on_segment = rects[2].Contains(io.MousePos);
					//timestamp selection
					if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						if (mouse_on_segment)
						{
							selected_timestamp = selected_timestamp_data
							{
								tag_info.name,
								&tag_info.timeline,
								timestamp_it
							};
						}

						deselect &= !mouse_on_segment;
					}

					const unsigned int quadColor[] = { 0xFFFFFFFF, 0xFFFFFFFF, slot_color/* + (selected ? 0 : 0x202020)*/};
					if (!segment_moving_data.has_value())// TODOFOCUS and backgroundRect.Contains(io.MousePos))
					{
						for (int j = 2; j >= 0; j--)
						{
							ImRect& rc = rects[j];
							if (!rc.Contains(io.MousePos))
								continue;
							ImGuiMouseCursor cursor = ImGui::GetMouseCursor();
							if ((j == 0 or j == 1) and timestamp_it->type() != tag_timestamp_type::point)
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
							if (ImGui::IsMouseClicked(0) and !moving_scroll_bar and !moving_time_marker and (timestamp_it->type() != tag_timestamp_type::point or j == 2))
							{
								segment_moving_data = moving_tag_data{
									&tag_info,
									timestamp_it,
									static_cast<uint8_t>(j + 1),
									mouse_pos_to_timestamp(io.MousePos.x),
									timestamp_it->start,
									timestamp_it->end
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

				// Tag segment context dots_hor
				//TODO: improve
				ImVec2 rp(canvas_pos.x, contentMin.y + ItemHeight * i);
				ImRect tag_line_rect(rp + ImVec2(float(legendWidth), float(0.f)), rp + ImVec2(canvas_size.x, float(ItemHeight)));

				bool insert_segment = false;
				static timestamp inserted_segment_start{};
				static timestamp inserted_segment_end{};

				static timestamp mouse_timestamp;
				ImGui::SetCursorScreenPos(tag_line_rect.Min);
				std::string button_id = std::string("##TagContextMenuTrigger") + std::to_string(i);
				if (ImGui::InvisibleButton(button_id.c_str(), tag_line_rect.Max - tag_line_rect.Min, ImGuiButtonFlags_MouseButtonRight))
				{
					mouse_timestamp = mouse_pos_to_timestamp(io.MousePos.x);
				}


				if (ImGui::BeginPopupContextItem())
				{
					ImVec2 pos = ImVec2(contentMin.x + legendWidth - firstFrameUsed * framePixelWidth, contentMin.y + ItemHeight * i + 1);
					if (ImGui::MenuItem("Add timestamp here"))
					{
						inserted_segment_start = mouse_timestamp;
						inserted_segment_end = inserted_segment_start;
						insert_segment = true;
					}
					if (ImGui::MenuItem("Start segment here"))
					{
						//TODO: should draw a line or something so you know where you clicked
						inserted_segment_start = mouse_timestamp;
					}
					//TODO: probably should only be displayed after start was pressed
					if (ImGui::MenuItem("End segment here"))
					{
						inserted_segment_end = mouse_timestamp;
						insert_segment = true;
					}
					if (ImGui::MenuItem("Add timestamp at marker"))
					{
						inserted_segment_start = current_time;
						inserted_segment_end = inserted_segment_start;
						insert_segment = true;
					}
					if (ImGui::MenuItem("Start segment at marker"))
					{
						//TODO: should draw a line or something so you know where you clicked
						inserted_segment_start = current_time;
					}
					//TODO: probably should only be displayed after start was pressed
					if (ImGui::MenuItem("End segment at marker"))
					{
						inserted_segment_end = current_time;
						insert_segment = true;
					}
					//TODO: probably should only be displayed when hovering a timestamp
					if (ImGui::MenuItem("Delete timestamp"))
					{
						auto selected_timepoint = mouse_timestamp;
						auto it = tag_info.timeline.find(selected_timepoint);
						if (it != tag_info.timeline.end())
						{
							tag_info.timeline.erase(it);
							if (selected_timestamp.has_value() and selected_timestamp->timestamp_timeline == &tag_info.timeline and selected_timestamp->timestamp == it)
							{
								selected_timestamp.reset();
							}
						}
					}
					ImGui::EndPopup();
				}

				if (insert_segment)
				{
					tag_info.timeline.insert(timestamp{ inserted_segment_start }, timestamp{ inserted_segment_end });
					inserted_segment_start = timestamp{};
					inserted_segment_end = timestamp{};
					dirty_flag = true;
				}
			}

			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) and deselect)
			{
				selected_timestamp = std::nullopt;
			}


			if (segment_moving_data.has_value())
			{
				if (segment_moving_data->grab_part == 1 or segment_moving_data->grab_part == 2)
				{
					ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
				}
			}

			// moving
			if (/*backgroundRect.Contains(io.MousePos) and */segment_moving_data.has_value())
			{
#if IMGUI_VERSION_NUM >= 18723
				ImGui::SetNextFrameWantCaptureMouse(true);
#else
				ImGui::CaptureMouseFromApp();
#endif
				auto mouse_timestamp = mouse_pos_to_timestamp(io.MousePos.x);
				auto move_delta = mouse_timestamp - segment_moving_data->grab_position;

				//auto diff_sec = mouse_pos_x - segment_moving_data->position.count();
				//auto diffFrame = std::chrono::seconds{ int64_t(diff_sec / framePixelWidth) };
				if (std::abs(move_delta.seconds_total.count()) > 0 and (state.time_min <= mouse_timestamp and mouse_timestamp <= state.time_max))
				{
					/*if (selected_entry)
						*selected_entry = movingEntry;*/

					//TODO: Maybe this should be shared between the timeline and the inspector since its also "used" there?
					constexpr auto min_segment_size = std::chrono::seconds{ 1 };

					if (segment_moving_data->grab_part & 1)
						segment_moving_data->left_position += move_delta;
					if (segment_moving_data->grab_part & 2)
						segment_moving_data->right_position += move_delta;
					if (segment_moving_data->left_position < timestamp{0})
					{
						if (segment_moving_data->grab_part & 2)
							segment_moving_data->right_position -= segment_moving_data->left_position;
						segment_moving_data->left_position = timestamp{0};
					}
					if (segment_moving_data->grab_part & 1 and segment_moving_data->left_position > segment_moving_data->right_position)
						segment_moving_data->left_position = segment_moving_data->right_position;
					if (segment_moving_data->grab_part & 2 and segment_moving_data->right_position < segment_moving_data->left_position)
						segment_moving_data->right_position = segment_moving_data->left_position;

					auto segment_size = std::abs((segment_moving_data->right_position - segment_moving_data->left_position).seconds_total.count());
					if (segment_size < min_segment_size.count() and segment_moving_data->segment->type() != tag_timestamp_type::point)
					{
						if (segment_moving_data->grab_part & 1)
						{
							segment_moving_data->left_position -= timestamp(min_segment_size - static_cast<std::chrono::seconds>(segment_size));
						}
						else if (segment_moving_data->grab_part & 2)
						{
							segment_moving_data->right_position += timestamp(min_segment_size - static_cast<std::chrono::seconds>(segment_size));
						}
					}
					segment_moving_data->grab_position = mouse_timestamp;
				}
				if (!io.MouseDown[0])
				{
					// single select
					//if (!diffFrame and movingPart and selected_entry)
					//{
					//	*selected_entry = movingEntry;
					//	ret = true;
					//}

					//TODO: If tags were to overlap, display a popup asking whether to merge the tags or not.
					auto& timeline = segment_moving_data->tag->timeline;

					bool was_selected = selected_timestamp.has_value() and selected_timestamp->timestamp_timeline == &timeline and selected_timestamp->timestamp == segment_moving_data->segment;

					if (selected_timestamp.has_value())
					{
						selected_timestamp->timestamp = timeline.replace
						(
							selected_timestamp->timestamp,
							timestamp{ segment_moving_data->left_position },
							timestamp{ segment_moving_data->right_position }
						).first;
					}
					segment_moving_data.reset();
					dirty_flag = true;
				}
			}
			draw_list->PopClipRect();
			draw_list->PopClipRect();

			// cursor
			if (current_time.seconds_total.count() >= state.first_frame and current_time.seconds_total.count() <= time_max)
			{
				static constexpr float cursorWidth = 4.f;
				static constexpr float triangle_span = cursorWidth * 2;
				float cursorOffset = contentMin.x + legendWidth + (current_time.seconds_total.count() - firstFrameUsed) * framePixelWidth + framePixelWidth / 2 - cursorWidth * 0.5f;
				ImU32 cursor_color = 0xE33E36FF; //0xA02A2AFF
				draw_list->AddLine(ImVec2(cursorOffset, canvas_pos.y), ImVec2(cursorOffset, contentMax.y), cursor_color, cursorWidth);
				draw_list->AddTriangleFilled(ImVec2(cursorOffset - triangle_span, canvas_pos.y), ImVec2(cursorOffset, canvas_pos.y + ItemHeight * 0.5f), ImVec2(cursorOffset + triangle_span, canvas_pos.y), cursor_color);
				/*
				char tmps[512];
				ImFormatString(tmps, IM_ARRAYSIZE(tmps), "%d", *current_time);
				draw_list->AddText(ImVec2(cursorOffset + 10, canvas_pos.y + 2), 0xFF2A2AFF, tmps);
				*/
			}


			/*for (auto& customDraw : customDraws)
				state->custom_draw(customDraw.index, draw_list, customDraw.customRect, customDraw.legendRect, customDraw.clippingRect, customDraw.legendClippingRect);
			for (auto& customDraw : compactCustomDraws)
				state->custom_draw_compact(customDraw.index, draw_list, customDraw.customRect, customDraw.clippingRect);*/

			// copy paste
			/*if (sequence_options & ImSequencer::SEQUENCER_COPYPASTE)
			{
				ImRect rectCopy(ImVec2(contentMin.x + 100, canvas_pos.y + 2)
					, ImVec2(contentMin.x + 100 + 30, canvas_pos.y + ItemHeight - 2));
				bool inRectCopy = rectCopy.Contains(io.MousePos);
				
				ImU32 copy_color = ImGui::ColorConvertFloat4ToU32(style.Colors[inRectCopy ? ImGuiCol_Text : ImGuiCol_TextDisabled]);
				draw_list->AddText(rectCopy.Min, copy_color, "Copy");

				ImRect rectPaste(ImVec2(contentMin.x + 140, canvas_pos.y + 2)
					, ImVec2(contentMin.x + 140 + 30, canvas_pos.y + ItemHeight - 2));
				bool inRectPaste = rectPaste.Contains(io.MousePos);
				ImU32 paste_color = ImGui::ColorConvertFloat4ToU32(style.Colors[inRectPaste ? ImGuiCol_Text : ImGuiCol_TextDisabled]);
				draw_list->AddText(rectPaste.Min, paste_color, "Paste");

				if (inRectCopy and io.MouseReleased[0])
				{
					state->copy();
				}
				if (inRectPaste and io.MouseReleased[0])
				{
					state->paste();
				}
			}*/

			ImGui::EndChildFrame();
			ImGui::PopStyleColor();
			if (hasScrollBar)
			{
				ImU32 scroll_bg_color = ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_ScrollbarBg]);
				ImU32 scroll_bg_alt_color = ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_MenuBarBg]);
				ImU32 scroll_color = ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_ScrollbarGrab]);
				ImU32 scroll_hover_color = ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_ScrollbarGrabHovered]);
				ImU32 scroll_active_color = ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_ScrollbarGrabActive]);

				ImGui::InvisibleButton("scrollBar", scrollBarSize);
				ImVec2 scrollBarMin = ImGui::GetItemRectMin();
				ImVec2 scrollBarMax = ImGui::GetItemRectMax();

				// ratio = number of frames visible in control / number to total frames

				float startFrameOffset = ((float)(firstFrameUsed - time_min) / (float)frameCount) * (canvas_size.x - legendWidth);
				ImVec2 scrollBarA(scrollBarMin.x + legendWidth, scrollBarMin.y - 2);
				ImVec2 scrollBarB(scrollBarMin.x + canvas_size.x, scrollBarMax.y - 1);
				draw_list->AddRectFilled(scrollBarA, scrollBarB, scroll_bg_alt_color, 0);

				ImRect scrollBarRect(scrollBarA, scrollBarB);
				bool inScrollBar = scrollBarRect.Contains(io.MousePos);

				draw_list->AddRectFilled(scrollBarA, scrollBarB, scroll_bg_color, style.ScrollbarRounding);


				ImVec2 scrollBarC(scrollBarMin.x + legendWidth + startFrameOffset, scrollBarMin.y);
				ImVec2 scrollBarD(scrollBarMin.x + legendWidth + barWidthInPixels + startFrameOffset, scrollBarMax.y - 2);
				draw_list->AddRectFilled(scrollBarC, scrollBarD, (inScrollBar || moving_scroll_bar) ? scroll_active_color : scroll_color, style.ScrollbarRounding);

				ImRect barHandleLeft(scrollBarC, ImVec2(scrollBarC.x + 14, scrollBarD.y));
				ImRect barHandleRight(ImVec2(scrollBarD.x - 14, scrollBarC.y), scrollBarD);

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
						framePixelWidthTarget = framePixelWidth = framePixelWidth / barRatio;
						int64_t newVisibleFrameCount = int64_t((canvas_size.x - legendWidth) / framePixelWidthTarget);
						int64_t lastFrame = state.first_frame + newVisibleFrameCount;
						if (lastFrame > time_max)
						{
							framePixelWidthTarget = framePixelWidth = (canvas_size.x - legendWidth) / float(time_max - state.first_frame);
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
							float previousFramePixelWidthTarget = framePixelWidthTarget;
							framePixelWidthTarget = framePixelWidth = framePixelWidth / barRatio;
							int64_t newVisibleFrameCount = int64_t(visibleFrameCount / barRatio);
							int64_t newFirstFrame = state.first_frame + newVisibleFrameCount - visibleFrameCount;
							newFirstFrame = std::clamp(newFirstFrame, time_min, std::max(time_max - visibleFrameCount, time_min));
							if (newFirstFrame == state.first_frame)
							{
								framePixelWidth = framePixelWidthTarget = previousFramePixelWidthTarget;
							}
							else
							{
								state.first_frame = newFirstFrame;
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
							state.first_frame = int64_t((io.MousePos.x - panningViewSource.x) / framesPerPixelInBar) - panningViewFrame;
							state.first_frame = std::clamp(state.first_frame, time_min, std::max(time_max - visibleFrameCount, time_min));
						}
					}
					else
					{
						if (scrollBarThumb.Contains(io.MousePos) and ImGui::IsMouseClicked(0) and !segment_moving_data.has_value())
						{
							moving_scroll_bar = true;
							panningViewSource = io.MousePos;
							panningViewFrame = -state.first_frame;
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

		//if (regionRect.Contains(io.MousePos))
		//{
		//	bool overCustomDraw = false;
		//	for (auto& custom : customDraws)
		//	{
		//		if (custom.customRect.Contains(io.MousePos))
		//		{
		//			overCustomDraw = true;
		//		}
		//	}
		//}


		//if (delEntry != -1)
		//{
		//	state.del(delEntry);
		//	if (selected_entry and (*selected_entry == delEntry || *selected_entry >= state.displayed_tags.size()))
		//		*selected_entry = -1;
		//}

		return return_value;
	}
}
