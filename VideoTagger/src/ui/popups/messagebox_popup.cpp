#include "messagebox_popup.hpp"
#include <ui/widgets/button_bar.hpp>
#include <core/app_context.hpp>
#include <utils/thumbnail.hpp>

namespace vt::ui
{
	messagebox_popup::messagebox_popup(std::optional<bool*> open) : modal_popup{ "messagebox", open, ImGuiWindowFlags_NoTitleBar }
	{
		
	}

	void messagebox_popup::set_data(const messagebox_data& data)
	{
		data_ = data;
	}

	void messagebox_popup::push_data(const messagebox_data& data)
	{
		message_queue_.push(data);
	}

	void messagebox_popup::pop_data()
	{
		if (!message_queue_.empty())
		{
			data_ = message_queue_.front();
			set_display_name(data_.title);
			message_queue_.pop();
		}
		else
		{
			close();
		}
	}

	bool messagebox_popup::should_open() const
	{
		return !is_open() and !message_queue_.empty();
	}

	void messagebox_popup::pre_style()
	{
		auto viewport = ImGui::GetMainViewport();
		auto min_width = viewport->Size.x * 0.2f;
		auto max_width = viewport->Size.x * 0.4f;
		auto max_height = viewport->Size.y * 0.35f;

		ImGui::SetNextWindowSizeConstraints({ min_width, 0.f }, { max_width, max_height });
	}

	void messagebox_popup::on_render()
	{
		const auto& style = ImGui::GetStyle();
		std::vector<std::pair<int, std::string>> buttons;
		for (const auto& button : data_.buttons)
		{
			buttons.push_back({ button.id, button.label });
		}

		if (data_.icon != messagebox_icon::none)
		{
			ImWchar icon_char{};
			ImVec4 icon_color{ 1.f, 1.f, 1.f, 1.f };
			switch (data_.icon)
			{
				case messagebox_icon::info:
				{
					icon_char = utils::thumbnail::info_icon;
					icon_color = ctx_.current_theme.get_float4(theme_color::common_info);
				}
				break;
				case messagebox_icon::warning:
				{
					icon_char = utils::thumbnail::warning_icon;
					icon_color = ctx_.current_theme.get_float4(theme_color::common_warning);
				}
				break;
				case messagebox_icon::error:
				{
					icon_char = utils::thumbnail::error_icon;
					icon_color = ctx_.current_theme.get_float4(theme_color::common_error);
				}
				break;
				default: break;
			}

			auto image = utils::thumbnail::font_texture();
			auto imgui_tex = reinterpret_cast<ImTextureID>((uintptr_t)image);
			auto glyph = utils::thumbnail::find_glyph(icon_char);
			auto icon_size = 32.f;
			ImGui::Image(imgui_tex, { icon_size, icon_size }, glyph.uv0, glyph.uv1, icon_color);
			ImGui::SameLine();
			ui::vertical_item_spacer(style.ItemSpacing.x * 2);
			ImGui::SameLine();
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + icon_size / 2.f - ImGui::GetTextLineHeight() / 2.f);
		}

		ImGui::TextWrapped("%s", data_.message.c_str());
		ui::vertical_item_spacer();

		ui::button_bar<int> bbar(buttons);
		bbar.set_default_button(data_.default_button_id);
		bbar.set_cancel_button(data_.cancel_button_id);
		bbar.render(0.f, true, [&](int id)
		{
			if (data_.callback)
			{
				data_.callback(id);
			}
			close();
		});
	}
}
