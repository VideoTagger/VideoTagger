#include "pch.hpp"
#include "text_input.hpp"
#include <widgets/controls.hpp>
#include <utils/string.hpp>
#include <ui/icons.hpp>
#include <ui/widgets/common.hpp>
#include <core/app_context.hpp>

namespace vt::ui
{
	text_input::text_input(const std::string& id, const std::string& hint, const std::function<std::optional<std::string>(const std::string& text)>& validator) : text_input{ id, {}, hint, validator } {}
	text_input::text_input(const std::string& id, const std::string& input, const std::string& hint, const std::function<std::optional<std::string>(const std::string& text)>& validator) : id_{ id }, input_{ input }, hint_{ hint }, validator_{ validator }, state_{ widget_state::normal }, is_password_{} {}
	
	void text_input::set_input(const std::string& input)
	{
		input_ = input;
	}

	void text_input::set_hint(const std::string& hint)
	{
		hint_ = hint;
	}

	void text_input::set_validator(const std::function<std::optional<std::string>(const std::string& text)>& validator)
	{
		validator_ = validator;
	}

	void text_input::set_is_password(bool value)
	{
		is_password_ = value;
	}

	void text_input::focus() const
	{
		ImGui::SetKeyboardFocusHere();
	}

	void text_input::clear()
	{
		input_.clear();
	}

	bool text_input::render()
	{
		const auto& style = ImGui::GetStyle();
		bool result{};
		int input_flags = ImGuiInputTextFlags_None;
		if (is_password_)
		{
			input_flags |= ImGuiInputTextFlags_Password;
		}

		auto val_result = validator_ != nullptr ? validator_(input_) : std::nullopt;
		bool valid = !val_result.has_value();
		//widgets::color_indicator(3.0f, valid ? ImGui::ColorConvertFloat4ToU32({0.15f, 0.75f, 0.15f, 1.f}) : 0xFF3E36FF);
		//ImGui::SameLine();

		ImVec4 bg_color = ImVec4{ 0.1765f, 0.1765f, 0.1765f, 1.f };
		if (state_ == widget_state::active)
		{
			bg_color = ImVec4{ 0.1216f, 0.1216f, 0.1216f, 1.f };
		}
		else if (state_ == widget_state::hovered)
		{
			bg_color = ImVec4{ 0.1961f, 0.1961f, 0.1961f, 1.f };
		}
		ImGui::PushStyleColor(ImGuiCol_FrameBg, bg_color);
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4{ 0.1882f, 0.1882f, 0.1882f, 1.f });
		ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4{});
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.f);

		if (!hint_.empty())
		{
			result = ImGui::InputTextWithHint(id_.c_str(), hint_.c_str(), &input_, input_flags);
		}
		else
		{
			result = ImGui::InputText(id_.c_str(), &input_, input_flags);
		}

		if (ImGui::IsItemFocused() and ImGui::IsItemActive())
		{
			state_ = widget_state::active;
		}
		else if (ImGui::IsItemHovered())
		{
			state_ = widget_state::hovered;
		}
		else
		{
			state_ = widget_state::normal;
		}

		static constexpr auto icon = icons::exclamation;
		auto icon_size = ImGui::CalcTextSize(icon);
		if (!valid and validator_ != nullptr)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{});
			ImGui::SameLine();
			ImGui::AlignTextToFramePadding();
			ImGui::TextColored(ctx_.current_theme.get_float4(theme_color::common_error), "%s", icon);
			ui::tooltip(val_result.value());
			ImGui::PopStyleVar();
		}
		else
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{});
			ImGui::SameLine();
			ImGui::Dummy(icon_size);
			ImGui::PopStyleVar();
		}
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);

		return result;
	}

	const std::string& text_input::input() const
	{
		return input_;
	}

	std::string text_input::trimmed_input() const
	{
		return utils::string::trim_whitespace(input());
	}

	[[nodiscard]] std::string text_input::error() const
	{
		if (validator_ == nullptr) return {};
		auto validation_result = validator_(input_);
		if (!validation_result.has_value()) return {};
		return validation_result.value();
	}

	bool text_input::is_valid() const
	{
		if (validator_ == nullptr) return true;
		return validator_(input_) == std::nullopt;
	}

	bool text_input::is_password() const
	{
		return is_password_;
	}

	text_input::operator bool() const
	{
		return is_valid();
	}
}
