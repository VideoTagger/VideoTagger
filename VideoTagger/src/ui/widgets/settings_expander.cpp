#include "pch.hpp"
#include "settings_expander.hpp"
#include <widgets/controls.hpp>

namespace vt::ui
{
	settings_expander::settings_expander(const std::string& header, const std::string& description, const std::function<void(float height)>& footer) : header_{ header }, description_{ description }, size_{}, footer_content_{ footer } {}

	bool settings_expander::render()
	{
		const auto& style = ImGui::GetStyle();

		auto cpos = ImGui::GetCursorScreenPos();
		cpos.x += style.FramePadding.x;

		float rounding = 4.0f;
		ImGui::GetWindowDrawList()->AddRectFilled(cpos, cpos + size_, IM_COL32(50, 50, 50, 100), rounding);

		widgets::horizontal_item_spacer(style.FramePadding.x);
		ImGui::SameLine();

		float width = ImGui::GetContentRegionAvail().x;
		ImVec2 table_size{ width, 0.f };
		if (ImGui::BeginTable("##SettingsExpander", 2, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_PadOuterX, table_size))
		{
			ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed);

			ImGui::TableNextColumn();
			ImGui::TextUnformatted(header_.c_str());
			if (!description_.empty())
			{
				ImGui::TextDisabled("%s", description_.c_str());
			}

			ImGui::TableNextColumn();
			if (footer_content_ != nullptr)
			{
				footer_content_(size_.y);
			}
			ImGui::SameLine();
			widgets::horizontal_item_spacer(style.FramePadding.x);
			ImGui::EndTable();
		}

		auto end_pos = ImGui::GetCursorScreenPos();
		end_pos.x += width - style.FramePadding.x;
		table_size.y = end_pos.y - cpos.y;
		size_ = table_size;
		return true;
	}
}
