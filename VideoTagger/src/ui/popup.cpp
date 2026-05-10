#include "pch.hpp"
#include "popup.hpp"

#include <core/app_context.hpp>
#include <ui/widgets/common.hpp>

namespace vt::ui
{
	popup::popup(const std::string& id, ImGuiWindowFlags flags) : id_{ id }, flags_{ flags }, imgui_id_{} {}

	popup::popup(const std::string& id, const std::string& display_name, ImGuiWindowFlags flags) : id_{ id }, display_name_{ display_name }, flags_ { flags }, imgui_id_{} {}

	void popup::open(ImGuiPopupFlags flags)
	{
		auto win_name = window_name();
		ImGui::OpenPopup(win_name.c_str(), flags);
	}

	void popup::close()
	{
		on_close();
		ImGui::CloseCurrentPopup();
	}

	void popup::set_display_name(const std::string& display_name)
	{
		display_name_ = display_name;
	}

	const std::string& popup::id() const
	{
		return id_;
	}

	const std::string& popup::display_name() const
	{
		return display_name_;
	}

	std::string popup::window_name() const
	{
		return fmt::format("{}###{}", display_name_, id_);
	}

	ImGuiWindowFlags popup::flags() const
	{
		return flags_;
	}

	void popup::on_display()
	{

	}

	void popup::on_close()
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
				auto disp_name = display_name();
				if (!disp_name.empty())
				{
					const auto& style = ImGui::GetStyle();
					ImGui::PushFont(ctx_.get_font(font_type::h3_bold));
					ImGui::TextUnformatted(disp_name.c_str());
					ImGui::PopFont();

					post_title_render();
					ui::vertical_item_spacer(ImGui::GetTextLineHeight() * 0.75f);
				}
			}
			on_render();
			post_render();
		}
	}

	bool popup::is_open() const
	{
		auto win_name = window_name();
		return ImGui::IsPopupOpen(win_name.c_str());
	}

	void popup::pre_style()
	{

	}

	void popup::post_style()
	{

	}

	bool popup::pre_render()
	{
		bool result;
		pre_style();
		auto win_name = window_name();
		result = ImGui::BeginPopup(win_name.c_str(), flags_);
		post_style();
		return result;
	}

	void popup::post_render()
	{
		ImGui::EndPopup();
	}

	void popup::post_title_render()
	{

	}

	modal_popup::modal_popup(const std::string& id, std::optional<bool*> open, ImGuiWindowFlags flags) : open_state_{ open }, popup{ id, flags } {}

    modal_popup::modal_popup(const std::string& id, const std::string& display_name, std::optional<bool*> open, ImGuiWindowFlags flags) : open_state_{ open }, popup{ id, display_name, flags } {}

    void modal_popup::close()
	{
		on_close();
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
		ui::begin_modal_style();
		auto flags_ = flags() | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;

		if (open_state_.has_value())
		{
			pre_style();
			auto win_name = window_name();
			result = ImGui::BeginPopupModal(win_name.c_str(), open_state_.value(), flags_);
			post_style();
		}
		else
		{
			bool v = true;
			pre_style();
			auto win_name = window_name();
			result = ImGui::BeginPopupModal(win_name.c_str(), &v, flags_);
			post_style();

			if (!v)
			{
				close();
			}
		}
		ui::end_modal_style();
		return result;
	}

	void modal_popup::post_title_render()
	{
		if (!open_state_.has_value()) return;

		const auto& style = ImGui::GetStyle();
		auto icon = icons::exit;
		ImGui::SameLine(ImGui::GetContentRegionMax().x - ImGui::CalcTextSize(icon).x - style.FramePadding.x - style.WindowPadding.x - style.WindowRounding);
		if (ui::icon_button(icon))
		{
			close();
		}
	}
}
