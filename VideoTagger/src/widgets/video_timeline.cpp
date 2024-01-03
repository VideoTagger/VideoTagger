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
#include "imgui.h"
#include "imgui_internal.h"

#include <algorithm>
#include <cstdlib>

namespace vt
{
#ifndef IMGUI_DEFINE_MATH_OPERATORS
	static ImVec2 operator+(const ImVec2& a, const ImVec2& b) {
		return ImVec2(a.x + b.x, a.y + b.y);
	}
#endif
	static bool SequencerAddDelButton(ImDrawList* draw_list, ImVec2 pos, bool add = true)
	{
		ImGuiIO& io = ImGui::GetIO();
		const ImGuiStyle& style = ImGui::GetStyle();
		ImRect btnRect(pos, ImVec2(pos.x + 16, pos.y + 16));
		bool overBtn = btnRect.Contains(io.MousePos);
		bool containedClick = overBtn && btnRect.Contains(io.MouseClickedPos[0]);
		bool clickedBtn = containedClick && io.MouseReleased[0];
		//int btnColor = overBtn ? 0xAAEAFFAA : 0x77A3B2AA;
		ImU32 button_color = ImGui::ColorConvertFloat4ToU32(style.Colors[overBtn ? ImGuiCol_Text : ImGuiCol_TextDisabled]);
		if (containedClick && io.MouseDownDuration[0] > 0)
			btnRect.Expand(2.0f);

		float midy = pos.y + 16 / 2 - 0.5f;
		float midx = pos.x + 16 / 2 - 0.5f;
		draw_list->AddRect(btnRect.Min, btnRect.Max, button_color, 4);
		draw_list->AddLine(ImVec2(btnRect.Min.x + 3, midy), ImVec2(btnRect.Max.x - 3, midy), button_color, 2);
		if (add)
			draw_list->AddLine(ImVec2(midx, btnRect.Min.y + 3), ImVec2(midx, btnRect.Max.y - 3), button_color, 2);
		return clickedBtn;
	}

	bool video_timeline(timeline_interface* sequence, timestamp* current_time, bool* expanded, int* selected_entry, int64_t* first_frame, int sequence_options)
	{
		bool ret = false;
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		int cx = (int)(io.MousePos.x);
		int cy = (int)(io.MousePos.y);
		static float framePixelWidth = 10.f;
		static float framePixelWidthTarget = 10.f;
		int legendWidth = 200;

		static int movingEntry = -1;
		static int movingPos = -1;
		static int movingPart = -1;
		int delEntry = -1;
		int dupEntry = -1;
		int ItemHeight = 20;

		const int64_t time_min = sequence->get_time_min().total_seconds.count();
		const int64_t time_max = sequence->get_time_max().total_seconds.count();

		bool popupOpened = false;
		int sequenceCount = sequence->get_item_count();
		if (!sequenceCount)
			return false;
		ImGui::BeginGroup();

		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 canvas_pos = ImGui::GetCursorScreenPos();            // ImDrawList API uses screen coordinates!
		ImVec2 canvas_size = ImGui::GetContentRegionAvail();        // Resize canvas to what's available
		int64_t firstFrameUsed = first_frame ? *first_frame : 0;


		int controlHeight = sequenceCount * ItemHeight;
		for (int i = 0; i < sequenceCount; i++)
			controlHeight += int(sequence->get_custom_height(i));
		int64_t frameCount = std::max(time_max - time_min, 1ll);

		static bool MovingScrollBar = false;
		static bool MovingCurrentFrame = false;
		struct CustomDraw
		{
			int index;
			ImRect customRect;
			ImRect legendRect;
			ImRect clippingRect;
			ImRect legendClippingRect;
		};
		ImVector<CustomDraw> customDraws;
		ImVector<CustomDraw> compactCustomDraws;
		// zoom in/out
		const int64_t visibleFrameCount = (int64_t)floorf((canvas_size.x - legendWidth) / framePixelWidth);
		const float barWidthRatio = std::min(visibleFrameCount / (float)frameCount, 1.f);
		const float barWidthInPixels = barWidthRatio * (canvas_size.x - legendWidth);

		ImRect regionRect(canvas_pos, canvas_pos + canvas_size);

		static bool panningView = false;
		static ImVec2 panningViewSource;
		static int64_t panningViewFrame;
		if (ImGui::IsWindowFocused() && io.KeyAlt && io.MouseDown[2])
		{
			if (!panningView)
			{
				panningViewSource = io.MousePos;
				panningView = true;
				panningViewFrame = *first_frame;
			}
			*first_frame = panningViewFrame - int64_t((io.MousePos.x - panningViewSource.x) / framePixelWidth);
			*first_frame = std::clamp(*first_frame, time_min, time_max - visibleFrameCount);
		}
		if (panningView && !io.MouseDown[2])
		{
			panningView = false;
		}
		framePixelWidthTarget = std::clamp(framePixelWidthTarget, 0.1f, 50.f);

		framePixelWidth = ImLerp(framePixelWidth, framePixelWidthTarget, 0.33f);

		frameCount = time_max - time_min;
		if (visibleFrameCount >= frameCount && first_frame)
			*first_frame = time_min;


		// --
		if (expanded && !*expanded)
		{
			ImGui::InvisibleButton("canvas", ImVec2(canvas_size.x - canvas_pos.x, (float)ItemHeight));
			draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_size.x + canvas_pos.x, canvas_pos.y + ItemHeight), 0xFF3D3837, 0);
			char tmps[512];
			ImFormatString(tmps, IM_ARRAYSIZE(tmps), sequence->get_collapse_fmt(), frameCount, sequenceCount);
			draw_list->AddText(ImVec2(canvas_pos.x + 26, canvas_pos.y + 2), 0xFFFFFFFF, tmps);
		}
		else
		{
			bool hasScrollBar(true);
			/*
			int framesPixelWidth = int(frameCount * framePixelWidth);
			if ((framesPixelWidth + legendWidth) >= canvas_size.x)
			{
				hasScrollBar = true;
			}
			*/
			// test scroll area
			ImVec2 headerSize(canvas_size.x, (float)ItemHeight);
			ImVec2 scrollBarSize(canvas_size.x, 14.f);
			ImGui::InvisibleButton("topBar", headerSize);
			draw_list->AddRectFilled(canvas_pos, canvas_pos + headerSize, 0xFFFF0000, 0);
			ImVec2 childFramePos = ImGui::GetCursorScreenPos();
			ImVec2 childFrameSize(canvas_size.x, canvas_size.y - 8.f - headerSize.y - (hasScrollBar ? scrollBarSize.y : 0));
			ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);
			ImGui::BeginChildFrame(889, childFrameSize);
			sequence->focused = ImGui::IsWindowFocused();
			ImGui::InvisibleButton("contentBar", ImVec2(canvas_size.x, float(controlHeight)));
			const ImVec2 contentMin = ImGui::GetItemRectMin();
			const ImVec2 contentMax = ImGui::GetItemRectMax();
			const ImRect contentRect(contentMin, contentMax);
			const float contentHeight = contentMax.y - contentMin.y;

			// full background
			
			ImU32 bg_color = ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_MenuBarBg]); //0xFF242424
			draw_list->AddRectFilled(canvas_pos, canvas_pos + canvas_size, bg_color, 0);

			// current frame top
			ImRect topRect(ImVec2(canvas_pos.x + legendWidth, canvas_pos.y), ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + ItemHeight));

			if (!MovingCurrentFrame && !MovingScrollBar && movingEntry == -1 && sequence_options & ImSequencer::SEQUENCER_CHANGE_FRAME && current_time && current_time->total_seconds.count() >= 0 && topRect.Contains(io.MousePos) && io.MouseDown[0])
			{
				MovingCurrentFrame = true;
			}
			if (MovingCurrentFrame)
			{
				if (frameCount)
				{
					current_time->total_seconds = std::chrono::seconds((int)((io.MousePos.x - topRect.Min.x) / framePixelWidth) + firstFrameUsed);
					if (current_time->total_seconds.count() < time_min)
						current_time->total_seconds = std::chrono::seconds(time_min);
					if (current_time->total_seconds.count() >= time_max)
						current_time->total_seconds = std::chrono::seconds(time_max);
				}
				if (!io.MouseDown[0])
					MovingCurrentFrame = false;
			}

			//header
			ImU32 header_color = ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_WindowBg]); //0xFF3D3837
			draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_size.x + canvas_pos.x, canvas_pos.y + ItemHeight), header_color, 0);
			if (sequence_options & ImSequencer::SEQUENCER_ADD)
			{
				if (SequencerAddDelButton(draw_list, ImVec2(canvas_pos.x + legendWidth - ItemHeight, canvas_pos.y + 2), true))
					ImGui::OpenPopup("addEntry");

				if (ImGui::BeginPopup("addEntry"))
				{
					for (int i = 0; i < sequence->get_item_type_count(); i++)
						if (ImGui::Selectable(sequence->get_item_type_name(i)))
						{
							sequence->add(i);
							*selected_entry = sequence->get_item_count() - 1;
						}

					ImGui::EndPopup();
					popupOpened = true;
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

			auto drawLine = [&](int64_t i, int regionHeight) {
				bool baseIndex = ((i % modFrameCount) == 0) || (i == time_max || i == time_min);
				bool halfIndex = (i % halfModFrameCount) == 0;
				int px = (int)canvas_pos.x + int(i * framePixelWidth) + legendWidth - int(firstFrameUsed * framePixelWidth);
				int tiretStart = baseIndex ? 4 : (halfIndex ? 10 : 14);
				int tiretEnd = baseIndex ? regionHeight : ItemHeight;

				if (px <= (canvas_size.x + canvas_pos.x) && px >= (canvas_pos.x + legendWidth))
				{
					draw_list->AddLine(ImVec2((float)px, canvas_pos.y + (float)tiretStart), ImVec2((float)px, canvas_pos.y + (float)tiretEnd - 1), 0xFF606060, 1);

					draw_list->AddLine(ImVec2((float)px, canvas_pos.y + (float)ItemHeight), ImVec2((float)px, canvas_pos.y + (float)regionHeight - 1), 0x30606060, 1);
				}

				if (baseIndex && px > (canvas_pos.x + legendWidth))
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

				if (px <= (canvas_size.x + canvas_pos.x) && px >= (canvas_pos.x + legendWidth))
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
			size_t customHeight = 0;
			for (int i = 0; i < sequenceCount; i++)
			{
				int type;
				sequence->get(i, NULL, NULL, &type, NULL);
				ImVec2 tpos(contentMin.x + 3, contentMin.y + i * ItemHeight + 2 + customHeight);
				ImU32 text_color = ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_Text]); //0xFFFFFFFF
				draw_list->AddText(tpos, text_color, sequence->get_item_label(i));

				if (sequence_options & ImSequencer::SEQUENCER_DEL)
				{
					if (SequencerAddDelButton(draw_list, ImVec2(contentMin.x + legendWidth - ItemHeight + 2 - 10, tpos.y + 2), false))
						delEntry = i;

					if (SequencerAddDelButton(draw_list, ImVec2(contentMin.x + legendWidth - ItemHeight - ItemHeight + 2 - 10, tpos.y + 2), true))
						dupEntry = i;
				}
				customHeight += sequence->get_custom_height(i);
			}

			// slots background
			customHeight = 0;
			auto slots_color4 = style.Colors[ImGuiCol_WindowBg];
			slots_color4.w *= 0.5f;
			ImU32 slots_color = ImGui::ColorConvertFloat4ToU32(slots_color4); //0xFF413D3D

			for (int i = 0; i < sequenceCount; i++)
			{
				//TODO: Change this
				unsigned int col = (i & 1) ? 0xFF3A3636 : 0xFF413D3D;
				//ImU32 col = slots_color;

				size_t localCustomHeight = sequence->get_custom_height(i);
				ImVec2 pos = ImVec2(contentMin.x + legendWidth, contentMin.y + ItemHeight * i + 1 + customHeight);
				ImVec2 sz = ImVec2(canvas_size.x + canvas_pos.x, pos.y + ItemHeight - 1 + localCustomHeight);
				if (!popupOpened && cy >= pos.y && cy < pos.y + (ItemHeight + localCustomHeight) && movingEntry == -1 && cx>contentMin.x && cx < contentMin.x + canvas_size.x)
				{
					col += 0x80201008;
					pos.x -= legendWidth;
				}
				draw_list->AddRectFilled(pos, sz, col, 0);
				customHeight += localCustomHeight;
			}

			draw_list->PushClipRect(childFramePos + ImVec2(float(legendWidth), 0.f), childFramePos + childFrameSize, true);

			// vertical frame lines in content area
			for (int64_t i = time_min; i <= time_max; i += frameStep)
			{
				drawLineContent(i, int(contentHeight));
			}
			drawLineContent(time_min, int(contentHeight));
			drawLineContent(time_max, int(contentHeight));

			// selection
			bool selected = selected_entry && (*selected_entry >= 0);
			if (selected)
			{
				customHeight = 0;
				for (int i = 0; i < *selected_entry; i++)
					customHeight += sequence->get_custom_height(i);
				//TODO: Change color
				draw_list->AddRectFilled(ImVec2(contentMin.x, contentMin.y + ItemHeight * *selected_entry + customHeight), ImVec2(contentMin.x + canvas_size.x, contentMin.y + ItemHeight * (*selected_entry + 1) + customHeight), 0x801080FF, 1.f);
			}

			// slots
			customHeight = 0;
			for (int i = 0; i < sequenceCount; i++)
			{
				int* start, * end;
				unsigned int color;
				sequence->get(i, &start, &end, NULL, &color);
				size_t localCustomHeight = sequence->get_custom_height(i);

				ImVec2 pos = ImVec2(contentMin.x + legendWidth - firstFrameUsed * framePixelWidth, contentMin.y + ItemHeight * i + 1 + customHeight);
				ImVec2 slotP1(pos.x + *start * framePixelWidth, pos.y + 2);
				ImVec2 slotP2(pos.x + *end * framePixelWidth + framePixelWidth, pos.y + ItemHeight - 2);
				ImVec2 slotP3(pos.x + *end * framePixelWidth + framePixelWidth, pos.y + ItemHeight - 2 + localCustomHeight);
				unsigned int slotColor = color | 0xFF000000;
				unsigned int slotColorHalf = (color & 0xFFFFFF) | 0x40000000;

				if (slotP1.x <= (canvas_size.x + contentMin.x) && slotP2.x >= (contentMin.x + legendWidth))
				{
					draw_list->AddRectFilled(slotP1, slotP3, slotColorHalf, 2);
					draw_list->AddRectFilled(slotP1, slotP2, slotColor, 2);
				}
				if (ImRect(slotP1, slotP2).Contains(io.MousePos) && io.MouseDoubleClicked[0])
				{
					sequence->double_click(i);
				}
				// Ensure grabbable handles
				const float max_handle_width = slotP2.x - slotP1.x / 3.0f;
				const float min_handle_width = std::min(10.0f, max_handle_width);
				const float handle_width = std::clamp(framePixelWidth / 2.0f, min_handle_width, max_handle_width);
				ImRect rects[3] = { ImRect(slotP1, ImVec2(slotP1.x + handle_width, slotP2.y))
					, ImRect(ImVec2(slotP2.x - handle_width, slotP1.y), slotP2)
					, ImRect(slotP1, slotP2) };

				const unsigned int quadColor[] = { 0xFFFFFFFF, 0xFFFFFFFF, slotColor + (selected ? 0 : 0x202020) };
				if (movingEntry == -1 && (sequence_options & ImSequencer::SEQUENCER_EDIT_STARTEND))// TODOFOCUS && backgroundRect.Contains(io.MousePos))
				{
					for (int j = 2; j >= 0; j--)
					{
						ImRect& rc = rects[j];
						if (!rc.Contains(io.MousePos))
							continue;
						draw_list->AddRectFilled(rc.Min, rc.Max, quadColor[j], 2);
					}

					for (int j = 0; j < 3; j++)
					{
						ImRect& rc = rects[j];
						if (!rc.Contains(io.MousePos))
							continue;
						if (!ImRect(childFramePos, childFramePos + childFrameSize).Contains(io.MousePos))
							continue;
						if (ImGui::IsMouseClicked(0) && !MovingScrollBar && !MovingCurrentFrame)
						{
							movingEntry = i;
							movingPos = cx;
							movingPart = j + 1;
							sequence->begin_edit(movingEntry);
							break;
						}
					}
				}

				// custom draw
				if (localCustomHeight > 0)
				{
					ImVec2 rp(canvas_pos.x, contentMin.y + ItemHeight * i + 1 + customHeight);
					ImRect customRect(rp + ImVec2(legendWidth - (firstFrameUsed - time_min - 0.5f) * framePixelWidth, float(ItemHeight)),
						rp + ImVec2(legendWidth + (time_max - firstFrameUsed - 0.5f + 2.f) * framePixelWidth, float(localCustomHeight + ItemHeight)));
					ImRect clippingRect(rp + ImVec2(float(legendWidth), float(ItemHeight)), rp + ImVec2(canvas_size.x, float(localCustomHeight + ItemHeight)));

					ImRect legendRect(rp + ImVec2(0.f, float(ItemHeight)), rp + ImVec2(float(legendWidth), float(localCustomHeight)));
					ImRect legendClippingRect(canvas_pos + ImVec2(0.f, float(ItemHeight)), canvas_pos + ImVec2(float(legendWidth), float(localCustomHeight + ItemHeight)));
					customDraws.push_back({ i, customRect, legendRect, clippingRect, legendClippingRect });
				}
				else
				{
					ImVec2 rp(canvas_pos.x, contentMin.y + ItemHeight * i + customHeight);
					ImRect customRect(rp + ImVec2(legendWidth - (firstFrameUsed - time_min - 0.5f) * framePixelWidth, float(0.f)),
						rp + ImVec2(legendWidth + (time_max - firstFrameUsed - 0.5f + 2.f) * framePixelWidth, float(ItemHeight)));
					ImRect clippingRect(rp + ImVec2(float(legendWidth), float(0.f)), rp + ImVec2(canvas_size.x, float(ItemHeight)));

					compactCustomDraws.push_back({ i, customRect, ImRect(), clippingRect, ImRect() });
				}
				customHeight += localCustomHeight;
			}


			// moving
			if (/*backgroundRect.Contains(io.MousePos) && */movingEntry >= 0)
			{
#if IMGUI_VERSION_NUM >= 18723
				ImGui::SetNextFrameWantCaptureMouse(true);
#else
				ImGui::CaptureMouseFromApp();
#endif
				int diffFrame = int((cx - movingPos) / framePixelWidth);
				if (std::abs(diffFrame) > 0)
				{
					int* start, * end;
					sequence->get(movingEntry, &start, &end, NULL, NULL);
					if (selected_entry)
						*selected_entry = movingEntry;
					int& l = *start;
					int& r = *end;
					if (movingPart & 1)
						l += diffFrame;
					if (movingPart & 2)
						r += diffFrame;
					if (l < 0)
					{
						if (movingPart & 2)
							r -= l;
						l = 0;
					}
					if (movingPart & 1 && l > r)
						l = r;
					if (movingPart & 2 && r < l)
						r = l;
					movingPos += int(diffFrame * framePixelWidth);
				}
				if (!io.MouseDown[0])
				{
					// single select
					if (!diffFrame && movingPart && selected_entry)
					{
						*selected_entry = movingEntry;
						ret = true;
					}

					movingEntry = -1;
					sequence->end_edit();
				}
			}
			draw_list->PopClipRect();
			draw_list->PopClipRect();

			// cursor
			if (current_time && first_frame && current_time->total_seconds.count() >= *first_frame && current_time->total_seconds.count() <= time_max)
			{
				static constexpr float cursorWidth = 4.f;
				static constexpr float triangle_span = cursorWidth * 2;
				float cursorOffset = contentMin.x + legendWidth + (current_time->total_seconds.count() - firstFrameUsed) * framePixelWidth + framePixelWidth / 2 - cursorWidth * 0.5f;
				ImU32 cursor_color = 0xE33E36FF; //0xA02A2AFF
				draw_list->AddLine(ImVec2(cursorOffset, canvas_pos.y), ImVec2(cursorOffset, contentMax.y), cursor_color, cursorWidth);
				draw_list->AddTriangleFilled(ImVec2(cursorOffset - triangle_span, canvas_pos.y), ImVec2(cursorOffset, canvas_pos.y + ItemHeight * 0.5f), ImVec2(cursorOffset + triangle_span, canvas_pos.y), cursor_color);
				/*
				char tmps[512];
				ImFormatString(tmps, IM_ARRAYSIZE(tmps), "%d", *current_time);
				draw_list->AddText(ImVec2(cursorOffset + 10, canvas_pos.y + 2), 0xFF2A2AFF, tmps);
				*/
			}


			for (auto& customDraw : customDraws)
				sequence->custom_draw(customDraw.index, draw_list, customDraw.customRect, customDraw.legendRect, customDraw.clippingRect, customDraw.legendClippingRect);
			for (auto& customDraw : compactCustomDraws)
				sequence->custom_draw_compact(customDraw.index, draw_list, customDraw.customRect, customDraw.clippingRect);

			// copy paste
			if (sequence_options & ImSequencer::SEQUENCER_COPYPASTE)
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

				if (inRectCopy && io.MouseReleased[0])
				{
					sequence->copy();
				}
				if (inRectPaste && io.MouseReleased[0])
				{
					sequence->paste();
				}
			}

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
				draw_list->AddRectFilled(scrollBarC, scrollBarD, (inScrollBar || MovingScrollBar) ? scroll_active_color : scroll_color, style.ScrollbarRounding);

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
						int64_t lastFrame = *first_frame + newVisibleFrameCount;
						if (lastFrame > time_max)
						{
							framePixelWidthTarget = framePixelWidth = (canvas_size.x - legendWidth) / float(time_max - *first_frame);
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
							int64_t newFirstFrame = *first_frame + newVisibleFrameCount - visibleFrameCount;
							newFirstFrame = std::clamp(newFirstFrame, time_min, std::max(time_max - visibleFrameCount, time_min));
							if (newFirstFrame == *first_frame)
							{
								framePixelWidth = framePixelWidthTarget = previousFramePixelWidthTarget;
							}
							else
							{
								*first_frame = newFirstFrame;
							}
						}
					}
				}
				else
				{
					if (MovingScrollBar)
					{
						if (!io.MouseDown[0])
						{
							MovingScrollBar = false;
						}
						else
						{
							float framesPerPixelInBar = barWidthInPixels / (float)visibleFrameCount;
							*first_frame = int64_t((io.MousePos.x - panningViewSource.x) / framesPerPixelInBar) - panningViewFrame;
							*first_frame = std::clamp(*first_frame, time_min, std::max(time_max - visibleFrameCount, time_min));
						}
					}
					else
					{
						if (scrollBarThumb.Contains(io.MousePos) && ImGui::IsMouseClicked(0) && first_frame && !MovingCurrentFrame && movingEntry == -1)
						{
							MovingScrollBar = true;
							panningViewSource = io.MousePos;
							panningViewFrame = -*first_frame;
						}
						if (!sizingRBar && onRight && ImGui::IsMouseClicked(0))
							sizingRBar = true;
						if (!sizingLBar && onLeft && ImGui::IsMouseClicked(0))
							sizingLBar = true;

					}
				}
			}
		}

		ImGui::EndGroup();

		if (regionRect.Contains(io.MousePos))
		{
			bool overCustomDraw = false;
			for (auto& custom : customDraws)
			{
				if (custom.customRect.Contains(io.MousePos))
				{
					overCustomDraw = true;
				}
			}
			if (overCustomDraw)
			{
			}
			else
			{
#if 0
				frameOverCursor = *firstFrame + (int)(visibleFrameCount * ((io.MousePos.x - (float)legendWidth - canvas_pos.x) / (canvas_size.x - legendWidth)));
				//frameOverCursor = max(min(*firstFrame - visibleFrameCount / 2, frameCount - visibleFrameCount), 0);

				/**firstFrame -= frameOverCursor;
				*firstFrame *= framePixelWidthTarget / framePixelWidth;
				*firstFrame += frameOverCursor;*/
				if (io.MouseWheel < -FLT_EPSILON)
				{
					*firstFrame -= frameOverCursor;
					*firstFrame = int(*firstFrame * 1.1f);
					framePixelWidthTarget *= 0.9f;
					*firstFrame += frameOverCursor;
				}

				if (io.MouseWheel > FLT_EPSILON)
				{
					*firstFrame -= frameOverCursor;
					*firstFrame = int(*firstFrame * 0.9f);
					framePixelWidthTarget *= 1.1f;
					*firstFrame += frameOverCursor;
				}
#endif
			}
		}

		if (expanded)
		{
			if (SequencerAddDelButton(draw_list, ImVec2(canvas_pos.x + 2, canvas_pos.y + 2), !*expanded))
				*expanded = !*expanded;
		}

		if (delEntry != -1)
		{
			sequence->del(delEntry);
			if (selected_entry && (*selected_entry == delEntry || *selected_entry >= sequence->get_item_count()))
				*selected_entry = -1;
		}

		if (dupEntry != -1)
		{
			sequence->duplicate(dupEntry);
		}
		return ret;
	}
}
