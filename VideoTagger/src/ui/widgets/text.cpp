#include "pch.hpp"
#include "text.hpp"

namespace vt::ui
{
	text::text(const std::string& content) : content_{ content } {}
	
	bool text::render()
	{
		ImGui::TextUnformatted(content_.c_str());
		return true;
	}

	void text::set_content(const std::string& content)
	{
		content_ = content;
	}

	const std::string& text::content() const
	{
		return content_;
	}
}
