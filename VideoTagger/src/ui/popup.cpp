#include "pch.hpp"
#include "popup.hpp"

#include <core/app_context.hpp>
#include <widgets/controls.hpp>
#include <ui/widgets/common.hpp>

namespace vt::ui
{
	popup::popup(const std::string& id, ImGuiWindowFlags flags) : id_{ id }, flags_{ flags }, imgui_id_{} {}

	popup::popup(const std::string& id, const std::string& display_name, ImGuiWindowFlags flags) : id_{ id }, display_name_{ display_name }, flags_ { flags }, imgui_id_{} {}

	void popup::open(ImGuiPopupFlags flags)
	{
		ImGui::OpenPopup(id_.c_str(), flags);
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
				const auto& style = ImGui::GetStyle();

				ImGui::PushFont(ctx_.get_font(font_type::h3));
				ImGui::TextUnformatted(display_name().c_str());
				ImGui::PopFont();
				post_title_render();
				ui::vertical_item_spacer(ImGui::GetTextLineHeight() * 0.75f);
			}
			on_render();
			post_render();
		}
	}

	bool popup::is_open() const
	{
		return ImGui::IsPopupOpen(id_.c_str());
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
		if (display_name_.empty())
		{
			pre_style();
			result = ImGui::BeginPopup(id_.c_str(), flags_);
			post_style();
		}
		else
		{
			pre_style();
			result = ImGui::BeginPopup(fmt::format("{}###{}", display_name_, id_).c_str(), flags_);
			post_style();
		}
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
		const char* id_cstr = id().c_str();
		ui::begin_modal_style();
		auto flags_ = flags() | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;

		if (open_state_.has_value())
		{
			pre_style();
			result = ImGui::BeginPopupModal(id_cstr, open_state_.value(), flags_);
			post_style();
		}
		else
		{
			bool v = true;
			pre_style();
			result = ImGui::BeginPopupModal(id_cstr, &v, flags_);
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
