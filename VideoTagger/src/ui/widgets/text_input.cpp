#include "pch.hpp"
#include "text_input.hpp"
#include <widgets/controls.hpp>
#include <utils/string.hpp>

namespace vt::ui
{
	text_input::text_input(const std::string& id, const std::string& hint, const std::function<bool(const std::string& text)>& validator) : text_input{ id, {}, hint, validator } {}
	text_input::text_input(const std::string& id, const std::string& input, const std::string& hint, const std::function<bool(const std::string& text)>& validator) : id_{ id }, input_{ input }, hint_{ hint }, validator_{ validator } {}
	
	void text_input::set_input(const std::string& input)
	{
		input_ = input;
	}

	void text_input::set_hint(const std::string& hint)
	{
		hint_ = hint;
	}

	void text_input::set_validator(const std::function<bool(const std::string& text)>& validator)
	{
		validator_ = validator;
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
		bool result{};
		bool valid = is_valid();
		widgets::color_indicator(3.0f, valid ? ImGui::ColorConvertFloat4ToU32({0.15f, 0.75f, 0.15f, 1.f}) : 0xFF3E36FF);
		ImGui::SameLine();
		if (!hint_.empty())
		{
			result = ImGui::InputTextWithHint(id_.c_str(), hint_.c_str(), &input_);
		}
		else
		{
			result = ImGui::InputText(id_.c_str(), &input_);
		}
		if (!valid)
		{
			widgets::tooltip("Invalid input");
		}
		return result;
	}

	bool text_input::render_with_label(const std::string& label, bool sameline)
	{
		ImGui::TextUnformatted(label.c_str());
		if (sameline)
		{
			ImGui::SameLine();
		}
		return render();
	}

	const std::string& text_input::input() const
	{
		return input_;
	}

	std::string text_input::trimmed_input() const
	{
		return utils::string::trim_whitespace(input());
	}

	bool text_input::is_valid() const
	{
		return validator_(input_);
	}

	text_input::operator bool() const
	{
		return is_valid();
	}
}
