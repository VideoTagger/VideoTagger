#pragma once
#include <imgui.h>
#include <string>

namespace vt::widgets
{
	struct color_picker
	{
	private:
		std::string id;
		ImVec4 color_buffer;
		bool open{};

	public:
		bool render(const std::string& label, ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
		void set_opened(bool value);
		void set_color(const ImVec4& color);

		const ImVec4& color() const;
	};
}
