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

namespace vt::widgets
{
	tag& timeline_state::get(size_t index)
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

	void timeline_state::del(size_t index)
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

	bool merge_timestamps_popup(const std::string& id, bool& pressed_button)
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
			ImGui::Text("Do you want to merge the overlapping segments?");
			ImGui::TextDisabled("(Pressing \"No\" will move the currently dragged segment back to its original position)");
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

	bool video_timeline(timeline_state& state, std::optional<selected_segment_data>& selected_timestamp, std::optional<moving_segment_data>& moving_timestamp, bool& dirty_flag)
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

		float ItemHeight = 20 * font_scale;

		const int64_t time_min = state.time_min.seconds_total.count();
		const int64_t time_max = state.time_max.seconds_total.count();

		ImGui::BeginGroup();

		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 canvas_pos = ImGui::GetCursorScreenPos();            // ImDrawList API uses screen coordinates!
		ImVec2 canvas_size = ImGui::GetContentRegionAvail();        // Resize canvas to what's available
		int64_t firstFrameUsed = state.first_frame;
		ImVec2 headerSize(canvas_size.x, (float)ItemHeight);
		ImVec2 scrollBarSize(canvas_size.x, 14.f);
		bool hasScrollBar(true);

		float controlHeight = std::max(std::max(state.displayed_tags.size(), size_t{1}) * ItemHeight, ImGui::GetWindowSize().y - (hasScrollBar ? scrollBarSize.y : 0));
		int64_t frameCount = std::max<int64_t>(time_max - time_min, 1);

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

		// zoom in/out
		const int64_t visibleFrameCount = (int64_t)floorf((canvas_size.x - legendWidth) / framePixelWidth);
		const float barWidthRatio = std::min(visibleFrameCount / (float)frameCount, 1.f);
		const float barWidthInPixels = barWidthRatio * (canvas_size.x - legendWidth);

		ImRect region_rect(canvas_pos, canvas_pos + canvas_size);

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
			//ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);
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

			if (!moving_time_marker and !moving_scroll_bar and !moving_timestamp.has_value() and state.current_time.seconds_total.count() >= 0 and topRect.Contains(io.MousePos) and io.MouseDown[0] and !ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup | ImGuiPopupFlags_AnyPopupId))
			{
				moving_time_marker = true;
			}
			if (moving_time_marker)
			{
				if (frameCount)
				{
					state.current_time.seconds_total = std::chrono::seconds((int)((io.MousePos.x - topRect.Min.x) / framePixelWidth) + firstFrameUsed);
					if (state.current_time.seconds_total.count() < time_min)
						state.current_time.seconds_total = std::chrono::seconds(time_min);
					if (state.current_time.seconds_total.count() >= time_max)
						state.current_time.seconds_total = std::chrono::seconds(time_max);
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
					auto selected_tag = state.tags->end();

					//TODO: This should already be sorted, not sorted every frame
					std::sort(state.displayed_tags.begin(), state.displayed_tags.end());
					if (tag_menu(*state.tags, state.displayed_tags))
					{
						ImGui::CloseCurrentPopup();
					}
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
			}

			draw_list->PushClipRect(childFramePos + ImVec2(float(legendWidth), 0.f), childFramePos + childFrameSize, true);

			// vertical frame lines in content area
			for (int64_t i = time_min; i <= time_max; i += frameStep)
			{
				drawLineContent(i, int(contentHeight));
			}
			drawLineContent(time_min, int(contentHeight));
			drawLineContent(time_max, int(contentHeight));

			// slots
			bool deselect = selected_timestamp.has_value();
			static std::optional<size_t> mouse_tag_line_index;
			if (!ImGui::IsPopupOpen("##SegmentContextMenu"))
			{
				mouse_tag_line_index.reset();
			}

			for (size_t i = 0; i < state.displayed_tags.size(); i++)
			{
				tag& tag_info = state.get(i);
				//TODO: consider using at() instead but then an entry would need to be created somewhere first
				tag_timeline& segments = (*state.segments)[tag_info.name];

				for (auto segment_it = segments.begin(); segment_it != segments.end(); ++segment_it)
				{
					if (moving_timestamp.has_value() and *moving_timestamp->tag == tag_info and moving_timestamp->segment_it == segment_it)
					{
						continue;
					}

					auto& tag_segment = *segment_it;

					int64_t start = tag_segment.start.seconds_total.count();
					int64_t end = tag_segment.end.seconds_total.count();

					uint32_t timestamp_color = tag_info.color & 0x00FFFFFF | 0xFF000000;

					ImVec2 pos = ImVec2(contentMin.x + legendWidth - firstFrameUsed * framePixelWidth, contentMin.y + ItemHeight * i + 1);
					ImVec2 slot_p1(pos.x + start * framePixelWidth, pos.y + 2);
					ImVec2 slot_p2(pos.x + end * framePixelWidth + framePixelWidth, pos.y + ItemHeight - 2);
					//ImVec2 slotP3(pos.x + end * framePixelWidth + framePixelWidth, pos.y + ItemHeight - 2);

					//TODO: should be somewhere else
					uint32_t selection_color = ImGui::ColorConvertFloat4ToU32({ 1.f, 0xA5 / 255.f, 0.f, 1.f }); //0xFFA500FF
					float selection_thickness = 2.0f;

					bool is_selected = selected_timestamp.has_value() and selected_timestamp->segments == &segments and selected_timestamp->segment_it == segment_it;

					// Drawing
					if (slot_p1.x <= (canvas_size.x + contentMin.x) and slot_p2.x >= (contentMin.x + legendWidth))
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
						else if (tag_segment.type() == tag_segment_type::point)
						{
							ImVec2 pos = { (slot_p2.x + slot_p1.x) / 2, slot_p1.y + ItemHeight / 2 - 2 };
							ImVec2 p1 = { pos.x, pos.y - ItemHeight / 2 + 1 };
							ImVec2 p2 = { pos.x + (slot_p2.x - slot_p1.x) / 2, pos.y };
							ImVec2 p3 = { pos.x, pos.y + ItemHeight / 2 - 1 };
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
					const float handle_width = std::clamp(framePixelWidth / 2.0f, min_handle_width, max_handle_width);
					ImRect rects[3] = {
						ImRect(slot_p1, ImVec2(slot_p1.x + handle_width, slot_p2.y)),
						ImRect(ImVec2(slot_p2.x - handle_width, slot_p1.y), slot_p2),
						ImRect(slot_p1, slot_p2)
					};

					bool mouse_on_segment = rects[2].Contains(io.MousePos);
					// Timestamp selection
					if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) or ImGui::IsMouseClicked(ImGuiMouseButton_Right))
					{
						if (mouse_on_segment)
						{
							selected_timestamp = selected_segment_data
							{
								&tag_info,
								&segments,
								segment_it
							};
						}

						deselect &= !mouse_on_segment;
					}

					const unsigned int quadColor[] = { 0xFFFFFFFF, 0xFFFFFFFF, timestamp_color/* + (selected ? 0 : 0x202020)*/ };
					if (!moving_timestamp.has_value())// TODOFOCUS and backgroundRect.Contains(io.MousePos))
					{
						for (int j = 2; j >= 0; j--)
						{
							ImRect& rc = rects[j];
							if (!rc.Contains(io.MousePos))
								continue;
							ImGuiMouseCursor cursor = ImGui::GetMouseCursor();
							if ((j == 0 or j == 1) and segment_it->type() != tag_segment_type::point)
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
							if (ImGui::IsMouseClicked(0) and !moving_scroll_bar and !moving_time_marker and (segment_it->type() != tag_segment_type::point or j == 2))
							{
								moving_timestamp = moving_segment_data{
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
				if (moving_timestamp.has_value() and *moving_timestamp->tag == tag_info)
				{
					auto& tag_segment = *moving_timestamp->segment_it;

					int64_t start = moving_timestamp->start.seconds_total.count();
					int64_t end = moving_timestamp->end.seconds_total.count();
					ImVec2 pos = ImVec2(contentMin.x + legendWidth - firstFrameUsed * framePixelWidth, contentMin.y + ItemHeight * i + 1);
					ImVec2 slot_p1(pos.x + start * framePixelWidth, pos.y + 2);
					ImVec2 slot_p2(pos.x + end * framePixelWidth + framePixelWidth, pos.y + ItemHeight - 2);

					uint32_t timestamp_color = tag_info.color & 0x00FFFFFF | 0x80000000;
					//TODO: should be somewhere else
					uint32_t selection_color = ImGui::ColorConvertFloat4ToU32({ 1.f, 0xA5 / 255.f, 0.f, 1.f });
					float selection_thickness = 2.0f;

					if (slot_p1.x <= (canvas_size.x + contentMin.x) and slot_p2.x >= (contentMin.x + legendWidth))
					{
						if (tag_segment.type() == tag_segment_type::segment)
						{
							//draw_list->AddRectFilled(slotP1, slotP3, slotColorHalf, 2);
							draw_list->AddRectFilled(slot_p1, slot_p2, timestamp_color, 2);
							draw_list->AddRect(slot_p1, slot_p2, selection_color, 2, 0, selection_thickness);
						}
						else if (tag_segment.type() == tag_segment_type::point)
						{
							ImVec2 pos = { (slot_p2.x + slot_p1.x) / 2, slot_p1.y + ItemHeight / 2 - 2 };
							ImVec2 p1 = { pos.x, pos.y - ItemHeight / 2 + 1 };
							ImVec2 p2 = { pos.x + (slot_p2.x - slot_p1.x) / 2, pos.y };
							ImVec2 p3 = { pos.x, pos.y + ItemHeight / 2 - 1 };
							ImVec2 p4 = { pos.x - (slot_p2.x - slot_p1.x) / 2, pos.y };

							draw_list->AddQuadFilled(p1, p2, p3, p4, timestamp_color);
							draw_list->AddQuad(p1, p2, p3, p4, selection_color, selection_thickness);
						}
					}
				}

				ImVec2 rp = { canvas_pos.x, contentMin.y + ItemHeight * i };
				ImRect tag_line_rect = { rp + ImVec2(float(legendWidth), float(0.f)), rp + ImVec2(canvas_size.x, float(ItemHeight)) };

				if (!ImGui::IsPopupOpen("##SegmentContextMenu") and tag_line_rect.Contains(io.MousePos))
				{
					mouse_tag_line_index = i;
				}
			}

			//TODO: improve
			// Segment context menu
			{
				static timestamp inserted_segment_start{};
				static timestamp inserted_segment_end{};
				static tag* tag_info{};
				static int selected_tag{};

				tag_timeline* segments{};
				if (mouse_tag_line_index.has_value())
				{
					tag_info = &state.get(*mouse_tag_line_index);
					segments = &state.segments->at(tag_info->name);
				}

				bool insert_segment = false;
				bool insert_segment_with_popup = false;

				static timestamp mouse_timestamp;

				if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
				{
					mouse_timestamp = mouse_pos_to_timestamp(io.MousePos.x);
				}

				if (ImGui::BeginPopupContextItem("##SegmentContextMenu"))
				{
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
							insert_segment_with_popup = true;
						}
						if (ImGui::MenuItem("Add segment"))
						{
							inserted_segment_start = mouse_timestamp;
							inserted_segment_end = inserted_segment_start + timestamp(1);
							insert_segment_with_popup = true;
						}
						ImGui::SeparatorText("Marker");
						//TODO: maybe could use the moving timestamp
						if (ImGui::MenuItem("Add timestamp at marker"))
						{
							inserted_segment_start = state.current_time;
							inserted_segment_end = inserted_segment_start;
							insert_segment_with_popup = true;
						}
						if (ImGui::MenuItem("Add segment at marker"))
						{
							//TODO: should draw a line or something so you know where you clicked
							inserted_segment_start = state.current_time;
							inserted_segment_start = state.current_time + timestamp(1);
							insert_segment_with_popup = true;
						}
						if (ImGui::MenuItem("Start segment at marker"))
						{
							//TODO: should draw a line or something so you know where you clicked
							inserted_segment_start = state.current_time;
						}
						//TODO: probably should only be displayed after start was pressed
						if (ImGui::MenuItem("End segment at marker"))
						{
							inserted_segment_end = state.current_time;
							insert_segment_with_popup = true;
						}
					}
					else
					{
						if (ImGui::MenuItem((*segment_it)->type() == tag_segment_type::point ? "Delete timestamp" : "Delete segment"))
						{
							segments->erase(*segment_it);
							if (selected_timestamp.has_value() and selected_timestamp->segments == segments and selected_timestamp->segment_it == *segment_it)
							{
								selected_timestamp.reset();
							}
						}
					}
					ImGui::EndPopup();
				}

				if (insert_segment)
				{
					//TODO: this doesn't display the merge popup

					segments->insert(timestamp{ inserted_segment_start }, timestamp{ inserted_segment_end });
					inserted_segment_start = timestamp{};
					inserted_segment_end = timestamp{};
					dirty_flag = true;
				}

				if (insert_segment_with_popup)
				{
					if (segments != nullptr)
					{
						auto it = std::find(state.displayed_tags.begin(), state.displayed_tags.end(), tag_info->name);
						selected_tag = (it != state.displayed_tags.end()) ? it - state.displayed_tags.begin() : 0;
					}
					ImGui::OpenPopup("Add Segment");
				}
				
				
				
				if (insert_segment_popup("Add Segment", inserted_segment_start, inserted_segment_end,
					state.time_min.seconds_total.count(), state.time_max.seconds_total.count(), state.displayed_tags, selected_tag))
				{
					//TODO: this doesn't display the merge popup

					auto& segments = state.segments->at(state.displayed_tags[selected_tag]);
					segments.insert(timestamp{ inserted_segment_start }, timestamp{ inserted_segment_end });
					inserted_segment_start = timestamp{};
					inserted_segment_end = timestamp{};
					dirty_flag = true;
				}
			}

			if (ImGui::IsMouseHoveringRect(contentMin, contentMax) and ImGui::IsMouseClicked(ImGuiMouseButton_Left) and deselect)
			{
				selected_timestamp = std::nullopt;
			}

			if (moving_timestamp.has_value())
			{
				if (moving_timestamp->grab_part == 1 or moving_timestamp->grab_part == 2)
				{
					ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
				}
			}

			//debug::log(std::to_string(ImGui::IsPopupOpen("Merge Overlapping", ImGuiPopupFlags_AnyPopupLevel | ImGuiPopupFlags_)));

			// moving
			if (/*region_rect.Contains(io.MousePos) and*/ moving_timestamp.has_value() and !ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup))
			{
				ImGui::SetNextFrameWantCaptureMouse(true);
				auto mouse_timestamp = mouse_pos_to_timestamp(io.MousePos.x);
				auto move_delta = mouse_timestamp - moving_timestamp->grab_position;

				//auto diff_sec = mouse_pos_x - segment_moving_data->position.count();
				//auto diffFrame = std::chrono::seconds{ int64_t(diff_sec / framePixelWidth) };
				if (std::abs(move_delta.seconds_total.count()) > 0 and (state.time_min <= mouse_timestamp and mouse_timestamp <= state.time_max))
				{
					/*if (selected_entry)
						*selected_entry = movingEntry;*/

					//TODO: Maybe this should be shared between the timeline and the inspector since its also "used" there?
					constexpr auto min_segment_size = std::chrono::seconds{ 1 };

					if (moving_timestamp->grab_part & 1)
						moving_timestamp->start += move_delta;
					if (moving_timestamp->grab_part & 2)
						moving_timestamp->end += move_delta;
					if (moving_timestamp->start < timestamp{0})
					{
						if (moving_timestamp->grab_part & 2)
							moving_timestamp->end -= moving_timestamp->start;
						moving_timestamp->start = timestamp{0};
					}
					if (moving_timestamp->grab_part & 1 and moving_timestamp->start > moving_timestamp->end)
						moving_timestamp->start = moving_timestamp->end;
					if (moving_timestamp->grab_part & 2 and moving_timestamp->end < moving_timestamp->start)
						moving_timestamp->end = moving_timestamp->start;

					auto segment_size = std::abs((moving_timestamp->end - moving_timestamp->start).seconds_total.count());
					if (segment_size < min_segment_size.count() and moving_timestamp->segment_it->type() != tag_segment_type::point)
					{
						if (moving_timestamp->grab_part & 1)
						{
							moving_timestamp->start -= timestamp(min_segment_size - static_cast<std::chrono::seconds>(segment_size));
						}
						else if (moving_timestamp->grab_part & 2)
						{
							moving_timestamp->end += timestamp(min_segment_size - static_cast<std::chrono::seconds>(segment_size));
						}
					}
					moving_timestamp->grab_position = mouse_timestamp;
				}
				if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
				{
					auto& segments = state.segments->at(moving_timestamp->tag->name);

					//No idea what this was supposed to be used for
					//bool was_selected = selected_timestamp.has_value() and selected_timestamp->timestamp_timeline == &timeline and selected_timestamp->timestamp == segment_moving_data->segment;

					if (selected_timestamp.has_value())
					{
						auto overlapping = segments.find_range(moving_timestamp->start, moving_timestamp->end);

						bool insert_now = true;
						for (auto it = overlapping.begin(); it != overlapping.end(); ++it)
						{
							if (it != selected_timestamp->segment_it)
							{
								insert_now = false;
							}
						}

						if (insert_now)
						{
							if (selected_timestamp->segment_it->start != moving_timestamp->start or selected_timestamp->segment_it->end != moving_timestamp->end)
							{
								selected_timestamp->segment_it = segments.replace
								(
									selected_timestamp->segment_it,
									moving_timestamp->start,
									moving_timestamp->end
								).first;

								dirty_flag = true;
							}
							moving_timestamp.reset();
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
			if (merge_timestamps_popup("##MergeSegments", pressed_yes))
			{
				if (pressed_yes and moving_timestamp.has_value())
				{
					auto& segments = state.segments->at(moving_timestamp->tag->name);
					selected_timestamp->segment_it = segments.replace
					(
						selected_timestamp->segment_it,
						timestamp{ moving_timestamp->start },
						timestamp{ moving_timestamp->end }
					).first;

					dirty_flag = true;
				}
				
				moving_timestamp.reset();
			}
			
			// cursor
			if (state.current_time.seconds_total.count() >= state.first_frame and state.current_time.seconds_total.count() <= time_max)
			{
				static constexpr float cursorWidth = 4.f;
				static constexpr float triangle_span = cursorWidth * 2;
				float cursorOffset = contentMin.x + legendWidth + (state.current_time.seconds_total.count() - firstFrameUsed) * framePixelWidth + framePixelWidth / 2 - cursorWidth * 0.5f;
				ImU32 cursor_color = 0xE33E36FF; //0xA02A2AFF
				draw_list->AddLine(ImVec2(cursorOffset, canvas_pos.y), ImVec2(cursorOffset, contentMax.y), cursor_color, cursorWidth);
				draw_list->AddTriangleFilled(
					ImVec2(cursorOffset - triangle_span, canvas_pos.y),
					ImVec2(cursorOffset, canvas_pos.y + ItemHeight * 0.5f),
					ImVec2(cursorOffset + triangle_span, canvas_pos.y), cursor_color
				);
			}

			ImGui::EndChildFrame();
			//ImGui::PopStyleColor();
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
						if (scrollBarThumb.Contains(io.MousePos) and ImGui::IsMouseClicked(0) and !moving_timestamp.has_value())
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

		return return_value;
	}
}
