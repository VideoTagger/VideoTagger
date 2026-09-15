#pragma once
#include <string>
#include <ui/widget.hpp>

namespace vt::ui
{
	struct text : public widget
	{
	public:
		text(const std::string& content);

	private:
		std::string content_;

	public:
		virtual bool render() override;
		void set_content(const std::string& content);

		[[nodiscard]] const std::string& content() const;
	};
}
