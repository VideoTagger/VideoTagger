#include "pch.hpp"
#include "controls.hpp"
#include "icons.hpp"
#include "time_input.hpp"

namespace vt::widgets
{
	bool checkbox(const char* label, bool* value)
	{
		auto& style = ImGui::GetStyle();

		bool result{};
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3);
		result = ImGui::Checkbox(label, value);
		ImGui::PopStyleVar();
		return result;
	}

	bool icon_button(const char* label, const ImVec2& size, const ImVec4& color)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{});
		ImGui::PushStyleColor(ImGuiCol_Text, color);
		bool result = ImGui::Button(label, size);
		ImGui::PopStyleColor(2);
		if (!is_item_disabled() and ImGui::IsItemHovered())
		{
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
		}
		return result;
	}

	void tooltip(const char* text)
	{
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip | ImGuiHoveredFlags_DelayNormal) and ImGui::BeginTooltip())
		{
			ImGui::TextUnformatted(text);
			ImGui::EndTooltip();
		}
	}

	bool icon_toggle_button(const char* label, bool is_toggled, const ImVec2& size, const ImVec4& color)
	{
		bool result = icon_button(label, size, is_toggled ? color : ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
		if (!is_item_disabled() and ImGui::IsItemHovered())
		{
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
		}
		return result;
	}

	bool collapsing_header(const char* label, bool hide_background)
	{
		auto& style = ImGui::GetStyle();
		if (hide_background) ImGui::PushStyleColor(ImGuiCol_Header, ImVec4{});
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{});
		std::string node_id = "##Node" + std::string(label);
		auto cx = ImGui::GetCursorPosX();
		bool result = ImGui::TreeNodeEx(node_id.c_str(), ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen);
		
		int pop_count = 1;
		if (hide_background) ++pop_count;
		ImGui::PopStyleColor(pop_count);
		auto icon = result ? icons::expand_less : icons::expand_more;

		ImGui::SameLine();
		auto px = ImGui::GetCursorPosX();
		ImGui::SetCursorPosX(px - (px - cx) + style.ItemInnerSpacing.x);
		//ImGui::SameLine(ImGui::GetTreeNodeToLabelSpacing());
		ImGui::TextUnformatted(label);
		ImGui::SameLine(ImGui::GetContentRegionMax().x - style.FramePadding.x - ImGui::CalcTextSize(icon).x);
		ImGui::TextUnformatted(icon);
		return result;
	}

	void label(const char* label)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, {});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, {});
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0, ImGui::GetStyle().FramePadding.y });
		ImGui::Button(label);
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();
	}

	//Original Author: https://github.com/ocornut/imgui/issues/474#issuecomment-169480920
	//This is a modified version
	bool begin_button_dropdown(const char* label, ImVec2 button_size, float popup_height)
	{
		ImGui::SameLine(0.f, 0.f);

		ImGuiWindow* window = ImGui::GetCurrentWindow();
		auto& g = *GImGui;
		const ImGuiStyle& style = g.Style;

		ImVec2 cursor_pos = ImGui::GetCursorPos();

		ImVec2 size(20, button_size.y);
		bool pressed = ImGui::Button("##", size);

		bool is_popup_open = ImGui::IsPopupOpen(label);
		// Arrow
		ImVec2 center(window->Pos.x + cursor_pos.x + 10, window->Pos.y + cursor_pos.y + button_size.y / 2);
		float r = 5.f;
		//center.y -= r * 0.25f;

		ImVec2 a;
		ImVec2 b;
		ImVec2 c;

		if (!is_popup_open)
		{
			a = center + ImVec2(0, 1) * r;
			b = center + ImVec2(-0.866f, -0.5f) * r;
			c = center + ImVec2(0.866f, -0.5f) * r;
		}
		else
		{
			b = center + ImVec2(0, -1) * r;
			a = center + ImVec2(-0.866f, 0.5f) * r;
			c = center + ImVec2(0.866f, 0.5f) * r;
		}
		window->DrawList->AddTriangleFilled(a, b, c, ImGui::GetColorU32(ImGuiCol_Text));

		// Popup
		ImVec2 popup_pos;

		popup_pos.x = window->Pos.x + cursor_pos.x - button_size.x;
		popup_pos.y = window->Pos.y + cursor_pos.y + button_size.y;
		auto viewport = ImGui::GetWindowViewport();
		auto end_height = popup_pos.y + popup_height;
		if (popup_height != 0.0f and end_height > viewport->Size.y)
		{
			popup_pos.y = window->Pos.y + cursor_pos.y - popup_height;
		}
		ImGui::SetNextWindowPos(popup_pos);

		if (pressed)
		{
			ImGui::OpenPopup(label);
		}

		if (ImGui::BeginPopup(label))
		{
			ImGui::PushStyleColor(ImGuiCol_FrameBg, style.Colors[ImGuiCol_Button]);
			ImGui::PushStyleColor(ImGuiCol_WindowBg, style.Colors[ImGuiCol_Button]);
			ImGui::PushStyleColor(ImGuiCol_ChildBg, style.Colors[ImGuiCol_Button]);
			return true;
		}

		return false;
	}

	void end_button_dropdown()
	{
		ImGui::PopStyleColor(3);
		ImGui::EndPopup();
	}

	void help_marker(const char* description)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::TextDisabled(icons::help);
		if (ImGui::BeginItemTooltip())
		{
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(description);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
		ImGui::PopStyleVar();
	}
	
	void centered_text(const char* text, ImVec2 avail_area, ImVec2 offset)
	{
		//auto half_text_size = ImGui::CalcTextSize(text, nullptr, false, 3 * avail_area.x / 4) / 2;
		auto half_text_size = ImGui::CalcTextSize(text, nullptr, false, 3 * avail_area.x / 4) / 2;
		auto cpos = ImGui::GetCursorPos();
		ImGui::SetCursorPos(offset + avail_area / 2 - half_text_size);
		ImGui::BeginDisabled();
		ImGui::TextWrapped("%s", text);
		ImGui::EndDisabled();
		ImGui::SetCursorPos(cpos);
	}

	void clipped_text(const char* text, ImVec2 avail_area)
	{
		static auto get_text_size = [&](const char* text)
		{
			auto size = ImGui::CalcTextSize(text, nullptr, false, avail_area.x);
			return size;
		};

		ImVec2 text_size = get_text_size(text);

		std::string str = text;
		if (text_size.x <= avail_area.x and text_size.y <= avail_area.y)
		{
			ImGui::TextWrapped("%s", text);
			return;
		}

		while (!str.empty() and (text_size.x > avail_area.x or text_size.y > avail_area.y))
		{
			str.pop_back();
			std::string temp = str + "...";
			text_size = get_text_size(temp.c_str());
		}

		if (!str.empty())
		{
			std::string temp = str + "...";
			ImGui::TextWrapped("%s", temp.c_str());
		}
	}
	
	bool timestamp_control(const std::string& name, timestamp& timestamp, uint64_t min_timestamp, uint64_t max_timestamp, bool* was_activated, bool* was_released, bool fill_area)
	{
		bool result = false;
		auto cstr = name.c_str();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, ImGui::GetStyle().ItemSpacing.y });
		if (ImGui::Button(cstr))
		{
			timestamp = vt::timestamp(min_timestamp);
			result = true;
		}
		ImGui::SameLine();
		if (fill_area) ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
		auto time_input_id = "##TimestampCtrlInput" + name;
		result |= widgets::time_input(time_input_id.c_str(), &timestamp, 1.0f, min_timestamp, max_timestamp, utils::time::default_time_format, ImGuiSliderFlags_AlwaysClamp);
		if (was_activated != nullptr) *was_activated = ImGui::IsItemActivated();
		if (was_released != nullptr) *was_released = ImGui::IsItemDeactivated();
		if (fill_area) ImGui::PopItemWidth();
		ImGui::PopStyleVar();
		auto ctx_name = ("##TimestampCtrlCtx" + name);
		if (ImGui::BeginPopupContextItem(ctx_name.c_str()))
		{
			if (ImGui::MenuItem("Set Min"))
			{
				timestamp = vt::timestamp(min_timestamp);
				result = true;
			}
			if (ImGui::MenuItem("Set Max"))
			{
				timestamp = vt::timestamp(max_timestamp);
				result = true;
			}
			ImGui::EndPopup();
		}
		return result;
	}

	extern bool search_bar(const char* label, const char* hint, std::string& buffer, float width, bool enable_button, ImGuiInputTextFlags flags)
	{
		bool empty = buffer.empty();
		bool result = !empty;
		if (width == 0)
		{
			width = ImGui::GetContentRegionAvail().x;
		}

		ImGui::PushID(label);
		float text_width = ImGui::GetTextLineHeight() + ImGui::GetStyle().FramePadding.x * 2;
		ImGui::SetNextItemWidth(width - text_width);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{});
		ImGui::BeginGroup();
		result &= ImGui::InputTextWithHint(label, hint, &buffer, flags);

		ImGui::SameLine();
		if (!enable_button or empty) ImGui::BeginDisabled();
		result |= ImGui::Button(icons::search, { text_width, 0 });
		if (!enable_button or empty) ImGui::EndDisabled();
		ImGui::PopStyleVar();
		ImGui::EndGroup();
		ImGui::PopID();

		return result;
	}

	ImVec2 calc_selectable_tile_size(ImVec2 tile_size)
	{
		auto& style = ImGui::GetStyle();
		auto text_size = ImVec2{ 0, 2 * ImGui::GetTextLineHeight() };
		return tile_size + style.FramePadding + text_size;
	}

	bool tile(const std::string& label, ImVec2 tile_size, ImVec2 image_size, GLuint image, const std::function<void(const std::string&)> context_menu, const std::function<void(const std::string&)> drag_drop, ImVec2 uv0, ImVec2 uv1, bool is_selected)
	{
		bool result{};
		ImVec2 image_tile_size = ImVec2{ tile_size.x, tile_size.x } * 0.9f;

		auto& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{});
		auto imgui_tex = reinterpret_cast<ImTextureID>((uintptr_t)image);
		const char* id = label.c_str();
		ImGui::PushID(id);
		auto text_size = ImVec2{ 0, 2 * ImGui::GetTextLineHeight() };
		auto selectable_size = tile_size + style.FramePadding + text_size;
		ImVec2 cpos = ImGui::GetCursorPos() + (selectable_size - image_size - text_size) / 2;
		
		ImGui::BeginGroup();
		ImGui::Selectable("##TileButton", &is_selected, ImGuiSelectableFlags_AllowOverlap | ImGuiSelectableFlags_AllowDoubleClick, selectable_size);
		if (ImGui::IsItemHovered() and ImGui::IsMouseDoubleClicked(0))
		{
			result = true;
		}
		if (drag_drop != nullptr)
		{
			std::invoke(drag_drop, label);
		}
		auto char_size = ImGui::CalcTextSize("A");
		size_t max_chars = static_cast<size_t>(std::floor(text_size.y / char_size.y) * static_cast<size_t>(selectable_size.x / char_size.x));
		bool is_shortened = strlen(id) > max_chars;
		std::string short_label = is_shortened ? std::string(id, max_chars) + "..." : label;

		if (is_shortened and ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip | ImGuiHoveredFlags_DelayNormal) and ImGui::BeginTooltip())
		{
			ImGui::Text("%s", id);
			ImGui::EndTooltip();
		}

		if (context_menu != nullptr and ImGui::BeginPopupContextItem("##TileCtxMenu"))
		{
			std::invoke(context_menu, label);
			ImGui::EndPopup();
		}

		ImGui::SetCursorPos(std::exchange(cpos, ImGui::GetCursorPos()));
		ImGui::Image(imgui_tex, image_size, uv0, uv1);
		ImGui::Dummy({ 0, (image_tile_size.y - image_size.y) / 2.f });
		//widgets::clipped_text(id, { tile_size.x, text_size.y });
		//TODO: Text clipping, change widgets::clipped_text into this
		if (ImGui::BeginTable("##TextContainer", 1, ImGuiTableFlags_NoSavedSettings, { selectable_size.x, 0 }, selectable_size.x))
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::TextWrapped("%s", short_label.c_str());
			ImGui::EndTable();
		}
		ImGui::EndGroup();

		ImGui::SetCursorPos(cpos);
		ImGui::PopID();
		ImGui::PopStyleVar();

		return result;
	}

	bool selection_area(ImVec2& start_pos, ImVec2& end_pos, ImGuiMouseButton mouse_button)
	{
		const auto& clip_rect = ImGui::GetCurrentWindow()->ClipRect;
		if (ImGui::IsMouseClicked(mouse_button))
		{
			start_pos = ImGui::GetMousePos();
		}
		
		bool valid
		{
			start_pos != end_pos and
			start_pos.x >= clip_rect.Min.x and start_pos.x <= clip_rect.Max.x and
			start_pos.y >= clip_rect.Min.y and start_pos.y <= clip_rect.Max.y
		};
		if (ImGui::IsMouseDown(mouse_button))
		{
			end_pos = ImGui::GetMousePos();
			end_pos.x = std::clamp(end_pos.x, clip_rect.Min.x, clip_rect.Max.x);
			end_pos.y = std::clamp(end_pos.y, clip_rect.Min.y, clip_rect.Max.y);

			if (valid)
			{
				ImDrawList* draw_list = ImGui::GetForegroundDrawList(); //ImGui::GetWindowDrawList();
				draw_list->AddRect(start_pos, end_pos, ImGui::GetColorU32(IM_COL32(0, 130, 216, 255)));   // Border
				draw_list->AddRectFilled(start_pos, end_pos, ImGui::GetColorU32(IM_COL32(0, 130, 216, 50)));    // Background
			}
		}
		return ImGui::IsMouseReleased(mouse_button) and valid;
	}

    void color_indicator(float thickness, uint32_t color)
    {
		auto& style = ImGui::GetStyle();
		float sz = ImGui::GetTextLineHeight();
		auto* window = ImGui::GetCurrentWindow();
		const ImRect rect{ window->DC.CursorPos, window->DC.CursorPos + ImVec2{ thickness, sz + style.FramePadding.y * 2.0f } };
		ImGui::Dummy(ImVec2{ thickness - style.ItemSpacing.x, sz });

		ImGui::RenderFrame(rect.Min, rect.Max, color, false, style.FrameRounding);
    }

	bool begin_collapsible(const std::string& id, const std::string& label, ImGuiTreeNodeFlags flags, const char* icon, const std::optional<ImVec4>& icon_color, const std::function<void(void)>& on_dragdrop, const std::optional<size_t>& index)
	{
		const auto& style = ImGui::GetStyle();
		flags |= ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_ClipLabelForTrailingButton;

		ImGui::BeginGroup();
		if (icon != nullptr)
		{
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + style.ItemSpacing.x * 0.5f);
			ImGui::AlignTextToFramePadding();
			ImGui::TextColored(icon_color.has_value() ? icon_color.value() : style.Colors[ImGuiCol_Text], icon);
			ImGui::SameLine();
		}

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{});
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{});
		bool node_open = ImGui::TreeNodeEx(id.c_str(), flags);
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();

		if (on_dragdrop != nullptr)
		{
			on_dragdrop();
		}

		auto suffix_icon = node_open ? icons::expand_less : icons::expand_more;

		ImGui::SameLine(ImGui::GetTreeNodeToLabelSpacing());
		ImGui::TextUnformatted(label.c_str());
		if (index.has_value())
		{
			ImGui::SameLine();
			ImGui::TextDisabled("[%zu]", index.value());
		}
		ImGui::SameLine(ImGui::GetContentRegionMax().x - style.ItemSpacing.x - ImGui::CalcTextSize(suffix_icon).x);
		ImGui::TextUnformatted(suffix_icon);
		ImGui::Unindent();
		ImGui::EndGroup();

		return node_open;
	}

	void end_collapsible()
	{
		ImGui::Indent();
		ImGui::TreePop();
	}

	bool is_item_disabled()
	{
		ImGuiContext& g = *GImGui;
		return (g.CurrentItemFlags & ImGuiItemFlags_Disabled) != 0;
	}

	bool table_hovered_row_style()
	{
		bool row_hovered = ImGui::TableGetHoveredRow() == ImGui::TableGetRowIndex();
		if (row_hovered)
		{
			ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyleColorVec4(ImGuiCol_TableRowBgAlt)));
		}
		return row_hovered;
	}

	bool positon_control(utils::vec2<uint32_t>& pos, const utils::vec2<uint32_t>& max_size)
	{
		bool result{};
		const auto& style = ImGui::GetStyle();

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{ style.ItemSpacing.x / 2, style.CellPadding.y });
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{});

		float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 xysize = { line_height + 3.0f, line_height };

		ImGui::PushID(&pos);
		if (ImGui::BeginTable("##PositionControl", 2))
		{
			ImGui::TableNextColumn();
			if (ImGui::Button("X", xysize))
			{
				pos[0] = 0;
				result = true;
			}
			ImGui::SameLine();

			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			float x = (float)pos[0];
			if (ImGui::DragScalar("##x", ImGuiDataType_U32, &pos[0], 1.f, 0, &max_size[0], "%d", ImGuiSliderFlags_AlwaysClamp))
			{
				pos[0] = std::clamp(pos[0], 0u, max_size[0]);
				result = true;
			}

			ImGui::TableNextColumn();
			if (ImGui::Button("Y", xysize))
			{
				pos[1] = 0;
				result = true;
			}
			ImGui::SameLine();

			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			if (ImGui::DragScalar("##y", ImGuiDataType_U32, &pos[1], 1.f, 0, &max_size[1], "%d", ImGuiSliderFlags_AlwaysClamp))
			{
				pos[1] = std::clamp(pos[1], 0u, max_size[1]);
				result = true;
			}
			ImGui::EndTable();
		}
		ImGui::PopID();
		ImGui::PopStyleVar(2);
		return result;
	}
}
