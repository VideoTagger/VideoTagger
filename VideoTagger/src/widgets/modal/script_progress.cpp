#include "pch.hpp"
#include "script_progress.hpp"
#include <core/app_context.hpp>
#include <utils/string.hpp>

namespace vt::widgets::modal
{
	void script_progress::open()
	{
		ImGui::OpenPopup("##ScriptProgress");
	}

	void script_progress::render(bool& is_open)
	{
		if (!ctx_.script_handle.has_value()) return;

		ImGuiWindowClass window_class{};
		window_class.ViewportFlagsOverrideSet = ImGuiViewportFlags_NoAutoMerge | ImGuiViewportFlags_TopMost;
		ImGui::SetNextWindowClass(&window_class);

		auto& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 7);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.WindowPadding * 2);
		ImGui::SetNextWindowSize({ 350.f, 0.f }, ImGuiCond_Always);
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		auto flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;

		auto win_open = ImGui::BeginPopupModal("##ScriptProgress", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoResize);
		ImGui::PopStyleVar(2);

		if (win_open)
		{
			ImGuiWindow* win = ImGui::GetCurrentWindow();
			auto wind = (SDL_Window*)ImGui::GetCurrentWindow()->Viewport->PlatformHandle;

			auto width = ImGui::GetContentRegionAvail().x;

			static uint8_t dot_count = 0;
			static constexpr uint8_t max_dots = 3;
			static float elapsed_acc{};

			auto elapsed_time = ImGui::GetIO().DeltaTime;

			if (elapsed_acc >= 0.75)
			{
				elapsed_acc = 0;
				dot_count = (dot_count + 1) % (max_dots + 1);
			}
			elapsed_acc += elapsed_time;

			if (std::shared_ptr<script> script = ctx_.script_handle->script.lock())
			{
				std::string suffix = std::string(dot_count, '.') + std::string(max_dots - dot_count, ' ');
				std::string info = utils::string::trim_whitespace(script->progress_info());

				ImGui::Text("%s%s", info.empty() ? "Script Running" : info.c_str(), suffix.c_str());
				if (script->has_progress())
				{
					ImGui::ProgressBar(script->progress(), ImVec2{ width, ImGui::GetTextLineHeight() / 3.f }, "");
				}
			}			

			if (ctx_.script_handle->has_finished())
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		if (ctx_.script_handle->has_finished())
		{
			ctx_.script_handle = std::nullopt;
			is_open = false;
		}
	}
}
