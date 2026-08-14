#include "pch.hpp"
#include "progress_popup.hpp"

#include <core/app_context.hpp>
#include <ui/widgets/button_bar.hpp>

namespace vt::ui
{
	progress_popup::progress_popup(const std::string& description, std::function<std::optional<float>(progress_popup&)> progress_callback, std::function<bool(const std::optional<float>& progress)> should_close, std::function<void()> on_cancel, std::optional<bool*> open) :
		modal_popup{ "progress", open, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoResize },
		description_{ description }, progress_callback_{ progress_callback }, should_close_{ should_close }, on_cancel_{ on_cancel }
	{
		if (progress_callback_ != nullptr)
		{
			progress_value_ = 0.f;
		}
	}

	void progress_popup::set_description(const std::string& description)
	{
		description_ = description;
	}

	void progress_popup::pre_style()
	{
		ImGui::SetNextWindowSize({ 350.f, 0.f }, ImGuiCond_Always);
	}

	void progress_popup::on_render()
	{
		if (progress_callback_ != nullptr)
		{
			progress_value_ = progress_callback_(*this);
		}

		if (should_close_(progress_value_))
		{
			close();
			return;
		}

		auto width = ImGui::GetContentRegionAvail().x;

		static constexpr uint8_t max_dots = 3;

		auto elapsed_time = ImGui::GetIO().DeltaTime;

		if (elapsed_acc_ >= 0.75)
		{
			elapsed_acc_ = 0;
			dot_count_ = (dot_count_ + 1) % (max_dots + 1);
		}
		elapsed_acc_ += elapsed_time;

		{
			auto& main_win = ctx_.main_window;
			auto taskbar = main_win->taskbar_proxy();

			std::string suffix = std::string(dot_count_, '.') + std::string(max_dots - dot_count_, ' ');

			ImGui::Text("%s%s", description_.c_str(), suffix.c_str());
			if (progress_value_.has_value())
			{
				ImGui::ProgressBar(*progress_value_, ImVec2{ width, ImGui::GetTextLineHeight() / 3.f }, "");
				taskbar.set_state(taskbar_state::normal);
				taskbar.set_value(*progress_value_, 1.0f, 0.f);
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
		ui::button_bar<int> button_bar{ buttons };
		button_bar.set_cancel_button(0);
		button_bar.render(0.f, true, [&](int id)
		{
			switch (id)
			{
			case 0:
			{
				if (on_cancel_ != nullptr)
				{
					on_cancel_();
				}
				close();
			}
			break;
			default: close(); break;
			}
		});
	}

	void progress_popup::on_close()
	{
		auto& main_win = ctx_.main_window;
		auto taskbar = main_win->taskbar_proxy();
		taskbar.reset();
	}
}
