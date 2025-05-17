#include "pch.hpp"
#include "popup.hpp"

#include <core/app_context.hpp>
#include <widgets/controls.hpp>

namespace vt::ui
{
	popup::popup(const std::string& id, ImGuiWindowFlags flags) : id_{ id }, flags_{ flags } {}

	void popup::open(ImGuiPopupFlags flags)
	{
		ImGui::OpenPopup(id_.c_str(), flags);
	}

	void popup::close()
	{
		ImGui::CloseCurrentPopup();
	}

	const std::string& popup::id() const
	{
		return id_;
	}

	ImGuiWindowFlags popup::flags() const
	{
		return flags_;
	}

	void popup::on_display()
	{

	}

    void popup::open_and_render(bool condition, ImGuiPopupFlags flags)
    {
		if (condition)
		{
			open(flags);
		}
		render();
    }

    void popup::render()
	{
		if (pre_render())
		{
			if (ImGui::IsWindowAppearing())
			{
				on_display();
			}

			if (flags() & ImGuiWindowFlags_NoTitleBar)
			{
				const auto& style = ImGui::GetStyle();

				ImGui::PushFont(ctx_.fonts["title"]);
				ImGui::TextUnformatted(id().c_str());
				ImGui::PopFont();
				post_title_render();
				widgets::vertical_item_spacer(ImGui::GetTextLineHeight() * 0.75f);
			}
			on_render();
			post_render();
		}
	}

	bool popup::pre_render()
	{
		return ImGui::BeginPopup(id_.c_str(), flags_);
	}

	void popup::post_render()
	{
		ImGui::EndPopup();
	}

	void popup::post_title_render()
	{

	}

	modal_popup::modal_popup(const std::string& id, std::optional<bool*> open, ImGuiWindowFlags flags) : open_state_{ open }, popup{ id, flags } {}

	void modal_popup::close()
	{
		if (open_state_.has_value() and *open_state_ != nullptr)
		{
			*open_state_.value() = false;
			ImGui::CloseCurrentPopup();
		}
		else
		{
			ImGui::CloseCurrentPopup();
		}
	}

	void modal_popup::close_on_escape()
	{
		if ((!open_state_.has_value() or open_state_.value() != nullptr) and ImGui::IsKeyPressed(ImGuiKey_Escape))
		{
			close();
		}
	}

	bool modal_popup::pre_render()
	{
		bool result{};
		const char* id_cstr = id().c_str();
		const auto& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.WindowPadding * 2);
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		auto flags_ = flags() | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;

		if (open_state_.has_value())
		{
			result = ImGui::BeginPopupModal(id_cstr, open_state_.value(), flags_);
		}
		else
		{
			bool v = true;
			result = ImGui::BeginPopupModal(id_cstr, &v, flags_);
			if (!v)
			{
				close();
			}
		}
		ImGui::PopStyleVar(2);
		return result;
	}

	void modal_popup::post_title_render()
	{
		if (!open_state_.has_value()) return;

		const auto& style = ImGui::GetStyle();
		auto icon = icons::exit;
		ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::CalcTextSize(icon).x - 2 * (style.WindowPadding.x - style.WindowRounding));
		if (widgets::icon_button(icon))
		{
			close();
		}
	}
}
