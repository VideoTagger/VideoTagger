#include "pch.hpp"
#include "insert_segment_popup.hpp"
#include "controls.hpp"

namespace vt::widgets
{
	bool insert_segment_popup(const char* id, timestamp& start, timestamp& end,
		uint64_t min_timestamp, uint64_t max_timestamp, const std::vector<std::string>& tags, int& selected_tag, bool& selected_ok)
	{
		bool result = false;
		auto& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 7);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.WindowPadding * 2);
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		auto flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;

		if (ImGui::BeginPopupModal(id, nullptr, flags))
		{
			std::vector<const char*> tags_cstr(tags.size());
			for (size_t i = 0; i < tags.size(); i++)
			{
				tags_cstr[i] = tags[i].c_str();
			}

			ImGui::Text("Tag");
			ImGui::Combo("##TagName", &selected_tag, tags_cstr.data(), static_cast<int>(tags_cstr.size()));
			widgets::timestamp_control("Start", start, min_timestamp, max_timestamp, nullptr, nullptr);
			widgets::timestamp_control("End", end, min_timestamp, max_timestamp, nullptr, nullptr);

			if (start > end)
			{
				std::swap(start, end);
			}

			if (ImGui::Button("OK") or ImGui::IsKeyPressed(ImGuiKey_Enter))
			{
				selected_ok = true;
				result = true;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel") or ImGui::IsKeyPressed(ImGuiKey_Escape))
			{
				selected_ok = false;
				result = true;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		ImGui::PopStyleVar(2);

		return result;
	}
}
