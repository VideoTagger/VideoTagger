#pragma once
#include <string>
#include <functional>

namespace vt::ui
{
	struct text_input
	{
	public:
		text_input(const std::string& id, const std::string& hint, const std::function<bool(const std::string& text)>& validator);
		text_input(const std::string& id, const std::string& input, const std::string& hint, const std::function<bool(const std::string& text)>& validator);

	private:
		std::string id_;
		std::string input_;
		std::string hint_;
		std::function<bool(const std::string& input)> validator_;

	public:
		void set_input(const std::string& input);
		void set_hint(const std::string& hint);
		void set_validator(const std::function<bool(const std::string& text)>& validator);
		void focus() const;
		void clear();
		bool render();
		bool render_with_label(const std::string& label, bool sameline = false);

		[[nodiscard]] const std::string& input() const;
		[[nodiscard]] std::string trimmed_input() const;
		[[nodiscard]] bool is_valid() const;
		operator bool() const;
	};
}
