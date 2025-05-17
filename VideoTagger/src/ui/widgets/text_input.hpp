#pragma once
#include <string>
#include <functional>
#include <ui/widget_state.hpp>
#include <ui/widget.hpp>

namespace vt::ui
{
	struct text_input : public widget
	{
	public:
		text_input(const std::string& id, const std::string& hint, const std::function<bool(const std::string& text)>& validator = nullptr);
		text_input(const std::string& id, const std::string& input, const std::string& hint, const std::function<bool(const std::string& text)>& validator = nullptr);

	private:
		std::function<bool(const std::string& input)> validator_;
		std::string id_;
		std::string input_;
		std::string hint_;
		widget_state state_;

	public:
		void set_input(const std::string& input);
		void set_hint(const std::string& hint);
		void set_validator(const std::function<bool(const std::string& text)>& validator = nullptr);
		void focus() const;
		void clear();
		virtual bool render() override;

		[[nodiscard]] const std::string& input() const;
		[[nodiscard]] std::string trimmed_input() const;
		[[nodiscard]] bool is_valid() const;
		operator bool() const;
	};
}
