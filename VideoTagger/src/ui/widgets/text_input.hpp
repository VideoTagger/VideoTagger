#pragma once
#include <string>
#include <functional>
#include <optional>
#include <ui/widget_state.hpp>
#include <ui/widget.hpp>

namespace vt::ui
{
	struct text_input : public widget
	{
	public:
		text_input(const std::string& id, const std::string& hint, const std::function<std::optional<std::string>(const std::string& text)>& validator = nullptr);
		text_input(const std::string& id, const std::string& input, const std::string& hint, const std::function<std::optional<std::string>(const std::string& text)>& validator = nullptr);

	private:
		std::function<std::optional<std::string>(const std::string& input)> validator_;
		std::string id_;
		std::string input_;
		std::string hint_;
		float width_;
		widget_state state_;
		ImGuiInputFlags flags_;

	public:
		void set_flags(ImGuiInputFlags flags);
		void set_input(const std::string& input);
		void set_hint(const std::string& hint);
		void set_validator(const std::function<std::optional<std::string>(const std::string& text)>& validator = nullptr);
		void set_is_password(bool value);
		void set_width(float width = 0.f);

		bool is_hovered() const;
		bool is_active() const;
		widget_state state() const;

		void focus() const;
		void clear();
		virtual bool render() override;

		[[nodiscard]] const std::string& input() const;
		[[nodiscard]] std::string trimmed_input() const;
		[[nodiscard]] std::string error() const;
		[[nodiscard]] bool is_valid() const;
		[[nodiscard]] bool is_password() const;
		operator bool() const;
	};
}
