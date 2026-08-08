#pragma once
#include <imgui.h>
#include <vector>
#include <sstream>
#include <string>
#include <functional>

#include <widgets/controls.hpp>
#include <ui/icons.hpp>
#include <ui/widget.hpp>
#include <ui/widgets/common.hpp>
#include <ui/widget_state.hpp>
#include <ui/impl/resettable.hpp>

namespace vt::ui
{
	template<typename value_type>
	struct combo : public widget, impl::resettable
	{
	public:
		constexpr combo(const std::string& id, const std::vector<value_type>& items, size_t selected = (size_t)(-1), const std::function<void(const std::pair<size_t, const value_type&>& item)>& callback = nullptr) : id_{ id }, items_{ items }, selected_{ selected }, callback_{ callback }, state_{ widget_state::normal }, available_width_{} {}

	private:
		std::vector<value_type> items_;
		std::function<void(const std::pair<size_t, const value_type&>& item)> callback_;
		std::string id_;
		float available_width_;
		size_t selected_;
		widget_state state_;

	public:
		constexpr void set_items(const std::vector<value_type>& items)
		{
			items_ = items;
			if (selected_ >= items_.size())
			{
				selected_ = 0;
			}
		}

		void set_callback(const std::function<void(const std::pair<size_t, const value_type&>& item)>& callback)
		{
			callback_ = callback;
		}

		constexpr void set_selected(size_t selected)
		{
			selected_ = std::clamp(selected, (size_t)0, items_.size() - 1);
		}

		///@brief Sets the available width for the combo box. When width > 0.f, the combo box will use all of the available content region width. (default: 0.f)
		constexpr void set_available_width(float width)
		{
			available_width_ = width;
		}

		[[nodiscard]] constexpr size_t selected() const
		{
			return selected_;
		}

		[[nodiscard]] constexpr const value_type& selected_item() const
		{
			return items_.at(selected_);
		}

		[[nodiscard]] constexpr const std::vector<value_type>& items() const
		{
			return items_;
		}

		[[nodiscard]] constexpr size_t item_count() const
		{
			return items_.size();
		}

		[[nodiscard]] constexpr bool empty() const
		{
			return items_.empty();
		}

		[[nodiscard]] constexpr const std::string& id() const
		{
			return id_;
		}

		virtual void reset() override
		{
			selected_ = 0;
			state_ = widget_state::normal;
		}

		virtual bool render() override
		{
			bool result{};
			const auto& style = ImGui::GetStyle();
			size_t item_count = items_.size();
			if (selected_ >= item_count)
			{
				selected_ = 0;
				if (callback_ != nullptr)
				{
					callback_({ selected_, items_.at(selected_) });
				}
			}

			std::stringstream ss;
			std::vector<std::string> labels(item_count);
			for (size_t i = 0; i < item_count; ++i)
			{
				const auto& ref = items_[i];
				ss.str("");
				ss << ref;

				labels[i] = ss.str();
			}

			ImVec4 bg_color = ImVec4{ 0.1765f, 0.1765f, 0.1765f, 1.f };
			if (state_ == widget_state::active)
			{
				bg_color = ImVec4{ 0.1216f, 0.1216f, 0.1216f, 1.f };
			}
			else if (state_ == widget_state::hovered)
			{
				bg_color = ImVec4{ 0.1961f, 0.1961f, 0.1961f, 1.f };
			}
			
			ImVec4 accent_color{ 0.2588f, 0.6f, 0.8784f, 1.f };
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.f);
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
			ImGui::PushStyleColor(ImGuiCol_FrameBg, bg_color);
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4{ 0.1882f, 0.1882f, 0.1882f, 1.f });
			ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4{});

			ImGui::PushItemWidth(available_width_ > 0.f ? available_width_ : ImGui::GetContentRegionAvail().x);
			bool open = ImGui::BeginCombo(id_.c_str(), labels[selected_].c_str(), ImGuiComboFlags_NoArrowButton);
			ImGui::PopStyleColor(3);
			ImGui::PopStyleVar(2);

			if (open)
			{
				for (size_t i = 0; i < item_count; ++i)
				{
					std::string label = labels[i];

					bool selected = (i == selected_);
					
					auto cpos = ImGui::GetCursorPos();

					ui::horizontal_item_spacer(3.f);
					ImGui::SameLine();
					if (ImGui::Selectable(label.c_str(), selected, ImGuiSelectableFlags_AllowOverlap | ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_SpanAvailWidth))
					{
						selected_ = i;
						result = true;
						if (callback_ != nullptr)
						{
							callback_({ selected_, items_.at(selected_) });
						}
					}

					auto post_cpos = ImGui::GetCursorPos();
					if (selected)
					{
						ImGui::SetItemDefaultFocus();
						
						cpos.x += style.FramePadding.x;
						cpos.y += style.FramePadding.y / 2.f;
						ImGui::SetCursorPos(cpos);
						ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.f);
						widgets::color_indicator(3.f, ImGui::ColorConvertFloat4ToU32(accent_color), 0.9f);
						ImGui::PopStyleVar();
						ImGui::SetCursorPos(post_cpos);
					}
				}
				ImGui::EndCombo();
			}

			if (open)
			{
				state_ = widget_state::active;
			}
			else if (ImGui::IsItemHovered())
			{
				state_ = widget_state::hovered;

				float wheel = ImGui::GetIO().MouseWheel;
				if (wheel != 0.0f and item_count > 1)
				{
					size_t old_selected = selected_;

					if (wheel > 0.0f and selected_ > 0)
					{
						--selected_;
					}
					else if (wheel < 0.0f and selected_ < item_count - 1)
					{
						++selected_;
					}

					if (selected_ != old_selected)
					{
						result = true;
						if (callback_ != nullptr)
						{
							callback_({ selected_, items_.at(selected_) });
						}
					}
				}
			}
			else
			{
				state_ = widget_state::normal;
			}

			auto cpos_x = ImGui::GetCursorPosX();
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{});
			ImGui::SameLine();
			ImGui::PopStyleVar();
			static constexpr auto icon = icons::expand_more;
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() - (ImGui::CalcTextSize(icon).x + style.FramePadding.x));
			ImGui::TextUnformatted(icon);

			ImGui::SetCursorPosX(cpos_x);
			return result;
		}

		constexpr static void render(const std::string& id, const std::vector<value_type>& items, size_t selected = (size_t)(-1), const std::function<void(const std::pair<size_t, const value_type&>& item)>& callback = nullptr)
		{
			combo cmb{ id, items, selected, callback };
			cmb.render();
		}
	};
}
