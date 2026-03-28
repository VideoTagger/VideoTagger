#include "pch.hpp"
#include "script_progress_popup.hpp"

#include <core/app_context.hpp>
#include <ui/widgets/button_bar.hpp>

namespace vt::ui
{
	script_progress_popup::script_progress_popup(std::optional<bool*> open) : modal_popup{ "script-progress", open, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoResize } {}

	void script_progress_popup::pre_style()
	{
		//ImGuiWindowClass window_class{};
		//window_class.ViewportFlagsOverrideSet = ImGuiViewportFlags_NoAutoMerge | ImGuiViewportFlags_TopMost;
		//ImGui::SetNextWindowClass(&window_class);

		//auto& style = ImGui::GetStyle();
		//ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 7);
		//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.WindowPadding * 2);
		ImGui::SetNextWindowSize({ 350.f, 0.f }, ImGuiCond_Always);
		//ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2{ 0.5f, 0.5f });
	}

	void script_progress_popup::post_style()
	{
		//ImGui::PopStyleVar(2);
	}

	void script_progress_popup::on_render()
	{
		std::shared_ptr<script> script = ctx_.script_handle->script.lock();
		if (!script)
		{
			return;
		}

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

		{
			auto& main_win = ctx_.main_window;
			auto taskbar = main_win->taskbar_proxy();

			py::gil_scoped_acquire lock{};
			std::string suffix = std::string(dot_count, '.') + std::string(max_dots - dot_count, ' ');
			std::string info = utils::string::trim_whitespace(script->progress_info());

			ImGui::Text("%s%s", info.empty() ? "Script Running" : info.c_str(), suffix.c_str());
			if (ctx_.script_handle->has_progress)
			{
				auto progress = script->progress();
				ImGui::ProgressBar(progress, ImVec2{ width, ImGui::GetTextLineHeight() / 3.f }, "");
				taskbar.set_state(taskbar_state::normal);
				taskbar.set_value(progress, 1.0f, 0.f);
			}
			else
			{
				taskbar.set_state(taskbar_state::indeterminate);
				ImGui::ProgressBar(-1.f * static_cast<float>(ImGui::GetTime()), ImVec2{ width, ImGui::GetTextLineHeight() / 3.f }, "");
			}
		}

		std::vector<std::pair<int, std::string>> buttons
		{
			{ 0, ctx_.lang->get("cancel") },
		};

		ui::button_bar<int>::render(buttons, [&](int id)
		{
			switch (id)
			{
				case 0:
				{
					ctx_.script_eng.interrupt();
					close();
				}
				break;
				default: close(); break;
			}
		});
	}

	void script_progress_popup::post_render()
	{
		modal_popup::post_render();

		if (is_open())
		{
			if (!ctx_.script_handle.has_value())
			{
				close();
				return;
			}

			if (ctx_.script_handle->has_finished())
			{
				debug::log("Closing script progress window...");
				ctx_.script_handle = std::nullopt;
				close();
				return;
			}
		}
	}
}
