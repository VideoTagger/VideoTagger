#include "time_input.hpp"
#include <cctype>
#include <sstream>
#include <iomanip>

#include <imgui_internal.h>

namespace vt::widgets
{
	static const float DRAG_MOUSE_THRESHOLD_FACTOR = 0.50f;

	static int input_text_callback(ImGuiInputTextCallbackData* data)
	{
		if (!(std::isdigit(data->EventChar) or data->EventChar == ':'))
		{
			return 1;
		}
		return 0;
	}

	//parses HH:MM:SS string into seconds
	uint64_t time_to_seconds(const std::string& input)
	{
		uint8_t n = 0;
		uint8_t mul = 0;
		uint64_t seconds{};
		uint64_t val{};
		auto it = input.rbegin();
		while (it != input.rend() and mul != 3)
		{
			char c = *it++;
			bool is_separator = (c == ':');
			if (!is_separator)
			{
				val += static_cast<uint64_t>(c - '0') * static_cast<uint64_t>(std::pow(10, n++));
			}
			if (is_separator or it == input.rend())
			{
				seconds += val * static_cast<uint64_t>(std::pow(60ull, mul++));
				n = 0;
				val = 0;
			}
		}
		return seconds;
	}

	static bool temp_input_text(const ImRect& bb, ImGuiID id, const char* label, char* buf, int buf_size, ImGuiInputTextFlags flags)
	{
		// On the first frame, g.TempInputTextId == 0, then on subsequent frames it becomes == id.
		// We clear ActiveID on the first frame to allow the InputText() taking it back.
		ImGuiContext& g = *GImGui;
		const bool init = (g.TempInputId != id);
		if (init)
			ImGui::ClearActiveID();

		g.CurrentWindow->DC.CursorPos = bb.Min;
		bool value_changed = ImGui::InputTextEx(label, NULL, buf, buf_size, bb.GetSize(), flags | ImGuiInputTextFlags_MergedItem | ImGuiInputTextFlags_CallbackCharFilter, input_text_callback);
		if (init)
		{
			// First frame we started displaying the InputText widget, we expect it to take the active id.
			IM_ASSERT(g.ActiveId == id);
			g.TempInputId = g.ActiveId;
		}
		return value_changed;
	}

	static bool temp_time_input_scalar(const ImRect& bb, ImGuiID id, const char* label, ImGuiDataType data_type, timestamp* p_data, const char* format, const void* p_clamp_min, const void* p_clamp_max)
	{
		// FIXME: May need to clarify display behavior if format doesn't contain %.
		// "%d" -> "%d" / "There are %d items" -> "%d" / "items" -> "%d" (fallback). Also see #6405
		const ImGuiDataTypeInfo* type_info = ImGui::DataTypeGetInfo(data_type);
		char data_buf[32];
		ImFormatString(data_buf, IM_ARRAYSIZE(data_buf), format, p_data->hours(), p_data->minutes(), p_data->seconds());
		ImStrTrimBlanks(data_buf);

		ImGuiInputTextFlags flags = ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_NoMarkEdited;

		bool value_changed = false;
		if (temp_input_text(bb, id, label, data_buf, IM_ARRAYSIZE(data_buf), flags))
		{

			// Backup old value
			size_t data_type_size = type_info->Size;
			ImGuiDataTypeTempStorage data_backup;
			memcpy(&data_backup, p_data, data_type_size);

			// Input text parsing
			p_data->seconds_total = std::chrono::seconds(time_to_seconds(data_buf));
			/*
			// Apply new value (or operations) then clamp
			ImGui::DataTypeApplyFromText(data_buf, data_type, p_data, format);
			if (p_clamp_min || p_clamp_max)
			{
				if (p_clamp_min && p_clamp_max && ImGui::DataTypeCompare(data_type, p_clamp_min, p_clamp_max) > 0)
					ImSwap(p_clamp_min, p_clamp_max);
				ImGui::DataTypeClamp(data_type, p_data, p_clamp_min, p_clamp_max);
			}
			*/

			// Only mark as edited if new value is different
			value_changed = memcmp(&data_backup, p_data, data_type_size) != 0;
			if (value_changed)
				ImGui::MarkItemEdited(id);
		}
		return value_changed;
	}

	bool time_input(const char* label, timestamp* v, float v_speed, uint64_t p_min, uint64_t p_max, const char* format, ImGuiSliderFlags flags)
	{
		ImGuiDataType data_type = ImGuiDataType_U64;
		uint64_t* p_data = reinterpret_cast<uint64_t*>(v);

		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);
		const float w = ImGui::CalcItemWidth();

		const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
		const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, label_size.y + style.FramePadding.y * 2.0f));
		const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));

		const bool temp_input_allowed = (flags & ImGuiSliderFlags_NoInput) == 0;
		ImGui::ItemSize(total_bb, style.FramePadding.y);
		if (!ImGui::ItemAdd(total_bb, id, &frame_bb, temp_input_allowed ? ImGuiItemFlags_Inputable : 0))
			return false;

		// Default format string when passing NULL
		if (format == NULL)
			format = ImGui::DataTypeGetInfo(data_type)->PrintFmt;

		const bool hovered = ImGui::ItemHoverable(frame_bb, id, g.LastItemData.InFlags);
		bool temp_input_is_active = temp_input_allowed && ImGui::TempInputIsActive(id);
		if (!temp_input_is_active)
		{
			// Tabbing or CTRL-clicking on Drag turns it into an InputText
			const bool input_requested_by_tabbing = temp_input_allowed && (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_FocusedByTabbing) != 0;
			const bool clicked = hovered && ImGui::IsMouseClicked(0, id);
			const bool double_clicked = (hovered && g.IO.MouseClickedCount[0] == 2 && ImGui::TestKeyOwner(ImGuiKey_MouseLeft, id));
			const bool make_active = (input_requested_by_tabbing || clicked || double_clicked || g.NavActivateId == id);
			if (make_active && (clicked || double_clicked))
				ImGui::SetKeyOwner(ImGuiKey_MouseLeft, id);
			if (make_active && temp_input_allowed)
				if (input_requested_by_tabbing || (clicked && g.IO.KeyCtrl) || double_clicked || (g.NavActivateId == id && (g.NavActivateFlags & ImGuiActivateFlags_PreferInput)))
					temp_input_is_active = true;

			// (Optional) simple click (without moving) turns Drag into an InputText
			if (g.IO.ConfigDragClickToInputText && temp_input_allowed && !temp_input_is_active)
				if (g.ActiveId == id && hovered && g.IO.MouseReleased[0] && !ImGui::IsMouseDragPastThreshold(0, g.IO.MouseDragThreshold * DRAG_MOUSE_THRESHOLD_FACTOR))
				{
					g.NavActivateId = id;
					g.NavActivateFlags = ImGuiActivateFlags_PreferInput;
					temp_input_is_active = true;
				}

			if (make_active && !temp_input_is_active)
			{
				ImGui::SetActiveID(id, window);
				ImGui::SetFocusID(id, window);
				ImGui::FocusWindow(window);
				g.ActiveIdUsingNavDirMask = (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
			}
		}

		if (temp_input_is_active)
		{
			// Only clamp CTRL+Click input when ImGuiSliderFlags_AlwaysClamp is set
			const bool is_clamp_input = (flags & ImGuiSliderFlags_AlwaysClamp) != 0 && (p_min == NULL || p_max == NULL || ImGui::DataTypeCompare(data_type, &p_min, &p_max) < 0);
			return temp_time_input_scalar(frame_bb, id, label, data_type, v, format, is_clamp_input ? &p_min : NULL, is_clamp_input ? &p_max : NULL);
		}

		// Draw frame
		const ImU32 frame_col = ImGui::GetColorU32(g.ActiveId == id ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
		ImGui::RenderNavHighlight(frame_bb, id);
		ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, frame_col, true, style.FrameRounding);

		// Drag behavior
		const bool value_changed = ImGui::DragBehavior(id, data_type, p_data, v_speed, &p_min, &p_max, format, flags);
		if (value_changed)
			ImGui::MarkItemEdited(id);

		// Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
		char value_buf[64];
		const char* value_buf_end = value_buf + ImFormatString(value_buf, IM_ARRAYSIZE(value_buf), format, v->hours(), v->minutes(), v->seconds());
		
		if (g.LogEnabled)
			ImGui::LogSetNextTextDecoration("{", "}");
		ImGui::RenderTextClipped(frame_bb.Min, frame_bb.Max, value_buf, value_buf_end, NULL, ImVec2(0.5f, 0.5f));

		if (label_size.x > 0.0f)
			ImGui::RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

		IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | (temp_input_allowed ? ImGuiItemStatusFlags_Inputable : 0));
		return value_changed;
	}
}
