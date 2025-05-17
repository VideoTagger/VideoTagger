#pragma once
#include <vector>
#include <functional>
#include <string>

#include <ui/widgets/common.hpp>
#include <ui/alignment.hpp>

namespace vt::ui
{
	template<typename id_type, bool enter_support = true>
	struct button_bar
	{
	public:
		constexpr button_bar(const std::vector<std::pair<id_type, std::string>>& buttons, horizontal_alignment alignment = horizontal_alignment::right) : buttons_{ buttons }, alignment_{ alignment } {}

	private:
		horizontal_alignment alignment_;
		std::vector<std::pair<id_type, std::string>> buttons_;

	public:
		constexpr void render(float available_width = 0.f, bool valid = true, const std::function<void(const id_type& id)>& callback = nullptr, bool last_default = false)
		{
			const auto& style = ImGui::GetStyle();
			size_t button_count = buttons_.size();
			if (button_count == 0) return;

			size_t default_button = last_default ? button_count - 1 : 0;

			if (available_width == 0.f)
			{
				available_width = ImGui::GetContentRegionAvail().x;
			}

			auto cpos_x = ImGui::GetCursorPosX();
			float spacing = style.ItemSpacing.x;
			float total_width{};

			for (const auto& [id, label] : buttons_)
			{
				total_width += ImGui::CalcTextSize(label.c_str()).x;
			}

			switch (alignment_)
			{
				case horizontal_alignment::center:
				{
					total_width += 2.f * spacing * (button_count - 1);
					ImGui::SetCursorPosX(available_width / 2.f - total_width / 2.f);
				}
				break;
				case horizontal_alignment::right:
				{
					total_width += 2.f * spacing * button_count;
					ImGui::SetCursorPosX(available_width - total_width);
				}
				break;
				default:
				{
					total_width += 2.f * spacing * button_count;
				}
				break;
			}

			for (size_t i = 0; i < button_count; ++i)
			{
				const auto& [id, label] = buttons_[i];
				if (i != 0)
				{
					ImGui::SameLine();
				}
				if (i == default_button)
				{
					ImGui::BeginDisabled(!valid);
					if constexpr (enter_support)
					{
						if (ui::accent_button(label) and callback != nullptr or (valid and callback != nullptr and ImGui::IsWindowFocused() and ImGui::IsKeyPressed(ImGuiKey_Enter)))
						{
							callback(id);
						}
					}
					else
					{
						if (ui::accent_button(label) and callback != nullptr)
						{
							callback(id);
						}
					}
					
					ImGui::EndDisabled();
				}
				else if (ui::button(label) and callback != nullptr)
				{
					callback(id);
				}
			}
			ImGui::SetCursorPosX(cpos_x);
		}

		constexpr static void render(const std::vector<std::pair<id_type, std::string>>& buttons, horizontal_alignment alignment = horizontal_alignment::right, float available_width = 0.f, bool valid = true, const std::function<void(const id_type& id)>& callback = nullptr)
		{
			button_bar bbar{ buttons, alignment };
			bbar.render(available_width, valid, callback);
		}

		constexpr static void render(const std::vector<std::pair<id_type, std::string>>& buttons, bool valid = true, const std::function<void(const id_type& id)>& callback = nullptr)
		{
			button_bar bbar{ buttons };
			bbar.render(0.f, valid, callback);
		}

		constexpr static void render(const std::vector<std::pair<id_type, std::string>>& buttons, const std::function<void(const id_type& id)>& callback = nullptr, bool last_default = false)
		{
			button_bar bbar{ buttons };
			bbar.render(0.f, true, callback, last_default);
		}
	};
}
