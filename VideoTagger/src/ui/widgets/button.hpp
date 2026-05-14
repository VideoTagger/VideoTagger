#pragma once
#include <string>
#include <ui/widget.hpp>
#include <imgui.h>

namespace vt::ui
{
	struct button : public widget
	{
	public:
		button(const std::string& label, const ImVec2& size = {});

	private:
		std::string label_;
		ImVec2 size_;

	public:
		virtual bool render() override;
	};
}
