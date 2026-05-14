#pragma once
#include <string>
#include <functional>

#include <ui/widget.hpp>

namespace vt::ui
{
	//TODO: Add expandible area
	struct settings_expander : public widget
	{
	public:
		settings_expander(const std::string& header, const std::string& description, const std::function<void(float height)>& footer);

	private:
		std::string header_;
		std::string description_;
		std::function<void(float height)> footer_content_;
		ImVec2 size_;

	public:
		virtual bool render() override;
	};
}
