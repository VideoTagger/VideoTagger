#include <pch.hpp>

#include "video_properties_popup.hpp"
#include <widgets/time_input.hpp>

namespace vt
{
	/*bool video_group_offset_popup(const std::string& id, const video_pool& videos, video_group& group)
	{
		bool result{};

		auto& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 7);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.WindowPadding * 2);
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		auto flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;
		bool is_open = ImGui::BeginPopupModal(id.c_str(), nullptr, flags);
		ImGui::PopStyleVar(2);

		bool is_appearing = ImGui::IsWindowAppearing();

		if (is_open)
		{
			if (!ImGui::BeginTable(nullptr, 2))
			{
				return false;
			}

			for (size_t i = 0; i < group.size(); i++)
			{
				ImGui::NextColumn();
				auto video_name = videos.get(group[i].id)->path.filename().u8string();
				ImGui::Text(video_name.c_str());

				ImGui::NextColumn();
				std::string input_id = "##Input" + std::string(id);
				timestamp ts(std::chrono::duration_cast<std::chrono::seconds>(group[i].offset));
				widgets::time_input(input_id.c_str(), &ts);
				group[i].offset = std::chrono::duration_cast<std::chrono::nanoseconds>(ts.seconds_total);
			}

			ImGui::EndTable();

			ImGui::Dummy(style.ItemSpacing);
			if (ImGui::Button("Create"))
			{
				result = true;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel") or ImGui::IsKeyPressed(ImGuiKey_Escape, false))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		return result;
	}*/

	bool video_properties_popup(const char* id, std::chrono::nanoseconds& offset)
	{
		bool result{};

		auto& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 7);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.WindowPadding * 2);
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		auto flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;
		bool is_open = ImGui::BeginPopupModal(id, nullptr, flags);
		ImGui::PopStyleVar(2);

		bool is_appearing = ImGui::IsWindowAppearing();

		if (is_open)
		{
			timestamp ts(std::chrono::duration_cast<std::chrono::milliseconds>(offset));
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted("Offset");
			ImGui::SameLine();
			widgets::time_input("##VideoOffsetInput", &ts);
			offset = std::chrono::duration_cast<std::chrono::nanoseconds>(ts.total_milliseconds);

			ImGui::Dummy(style.ItemSpacing);
			if (ImGui::Button("OK"))
			{
				result = true;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel") or ImGui::IsKeyPressed(ImGuiKey_Escape, false))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		return result;
	}
}
