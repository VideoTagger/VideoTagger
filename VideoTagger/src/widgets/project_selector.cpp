#define _CRT_SECURE_NO_WARNINGS
#include "project_selector.hpp"
#include <sstream>
#include <chrono>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>

std::chrono::system_clock::time_point to_sys_time(const std::filesystem::file_time_type& ftime)
{
	using namespace std::literals;
	return std::chrono::system_clock::time_point{ ftime.time_since_epoch() - 3234576h };
}

//Author: https://github.com/ocornut/imgui/issues/474#issuecomment-169480920
bool BeginButtonDropDown(const char* label, ImVec2 button_size)
{
	ImGui::SameLine(0.f, 0.f);

	ImGuiWindow* window = ImGui::GetCurrentWindow();
	auto& g = *GImGui;
	const ImGuiStyle& style = g.Style;

	ImVec2 cursor_pos = ImGui::GetCursorPos();

	ImVec2 size(20, button_size.y);
	bool pressed = ImGui::Button("##", size);

	// Arrow
	ImVec2 center(window->Pos.x + cursor_pos.x + 10, window->Pos.y + cursor_pos.y + button_size.y / 2);
	float r = 8.f;
	center.y -= r * 0.25f;
	ImVec2 a = center + ImVec2(0, 1) * r;
	ImVec2 b = center + ImVec2(-0.866f, -0.5f) * r;
	ImVec2 c = center + ImVec2(0.866f, -0.5f) * r;

	window->DrawList->AddTriangleFilled(a, b, c, ImGui::GetColorU32(ImGuiCol_Text));

	// Popup

	ImVec2 popup_pos;

	popup_pos.x = window->Pos.x + cursor_pos.x - button_size.x;
	popup_pos.y = window->Pos.y + cursor_pos.y + button_size.y;

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

void EndButtonDropDown()
{
	ImGui::PopStyleColor(3);
	ImGui::EndPopup();
}

namespace vt::widgets
{
	project_selector::project_selector(const std::vector<project>& projects) : projects_{ projects }
	{

	}

	void project_selector::render_project_widget(size_t id, const project& project)
	{
		ImVec2 size = { 0, ImGui::GetTextLineHeightWithSpacing() * 2 };
		auto imgui_id = static_cast<ImGuiID>(id);

		if (id == 0)
		{
			ImGui::TableSetupColumn("Project Name", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("Modification Time", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableHeadersRow();
		}
		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		ImGui::PushID(imgui_id);
		bool is_valid = project.is_valid();
		if (!is_valid)
		{
			ImGui::BeginDisabled();
		}
		if (ImGui::Selectable("##ProjectListSelectable", false, ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_SpanAllColumns, size) and on_click_project != nullptr)
		{
			on_click_project(project);
		}
		if (!is_valid)
		{
			ImGui::EndDisabled();
		}
		ImGui::PopID();

		ImGui::SameLine();

		ImGui::BeginGroup();
		std::string name = !project.name.empty() ? project.name : "Invalid Project!";
		ImGui::Text(name.c_str());
		auto path = project.path;
		std::filesystem::file_time_type mod_time{};
		if (!path.empty() and std::filesystem::exists(path))
		{
			mod_time = std::filesystem::last_write_time(path);
			path = std::filesystem::absolute(path);
		}
		ImGui::TextDisabled(path.string().c_str());
		ImGui::EndGroup();

		ImGui::TableNextColumn();
		std::string time_str;
		if (mod_time != std::filesystem::file_time_type{})
		{
			auto time = to_sys_time(mod_time);
			auto tt = std::chrono::system_clock::to_time_t(time);
			std::tm tm = *std::localtime(&tt);
			std::stringstream ss;
			ss << std::put_time(&tm, "%d-%m-%Y %H:%M:%S");
			time_str = ss.str();
		}
		
		ImGui::AlignTextToFramePadding();
		ImGui::Text(time_str.c_str());

		ImGui::TableNextColumn();
		ImGui::PushID(imgui_id);
		if (ImGui::Button("...", size))
		{
			ImGui::OpenPopup("##ProjectCtxMenu");
		}

		if (ImGui::BeginPopup("##ProjectCtxMenu"))
		{
			if (ImGui::MenuItem("Remove"))
			{
				projects_.erase(std::find(projects_.begin(), projects_.end(), project));
			}
			ImGui::EndPopup();
		}
		ImGui::PopID();
	}

	void project_selector::render()
	{
		if (ImGui::Begin("Project Selector", nullptr))
		{
			std::string buffer;
			ImGui::SeparatorText("Projects");

			auto content_width = ImGui::GetContentRegionMax().x;
			ImGui::SetNextItemWidth(content_width);
			ImGui::InputTextWithHint("##ProjectSelectorSearch", "Search...", &buffer);

			ImVec2 new_proj_size = { content_width, ImGui::GetTextLineHeightWithSpacing() * 2 };
			ImVec2 list_size = ImGui::GetContentRegionAvail();
			const auto& style = ImGui::GetStyle();
			list_size.y -= new_proj_size.y + style.ItemSpacing.y + style.WindowPadding.y;
			if (ImGui::BeginChild("##Project List", list_size))
			{
				if (ImGui::BeginTable("##ProjectListTable", 3, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerV))
				{
					for (size_t i = 0; i < projects_.size(); ++i)
					{
						render_project_widget(i, projects_[i]);
					}
					ImGui::EndTable();
				}
			}
			ImGui::EndChild();
			new_proj_size.x -= ImGui::GetTextLineHeightWithSpacing() + style.WindowPadding.x;

			ImGui::Dummy(style.ItemSpacing);
			if (ImGui::Button("New Project", new_proj_size))
			{
				projects_.push_back(projects_.back());
			}

			if (BeginButtonDropDown("##spdd", new_proj_size))
			{
				if (ImGui::Button("Add Existing Project", new_proj_size))
				{
					projects_.push_back(projects_.back());
				}
				EndButtonDropDown();
			}
		}
		ImGui::End();
	}
}
