#pragma once
#include <imgui.h>
#include <ui/widgets/slider.hpp>
#include <core/app_context.hpp>

namespace vt::ui
{
	template<typename type>
	struct themed_slider : public slider<type>
	{
	public:
		constexpr themed_slider(type min = std::numeric_limits<type>::min(), type max = std::numeric_limits<type>::max(), type value = {}, const ImVec2& size = {}) : slider<type>{ min, max, value, size } {}

	public:
		virtual void pre_style() override
		{
			auto& style = ImGui::GetStyle();
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.f);
			ImGui::PushStyleColor(ImGuiCol_SliderGrab, ctx_.current_theme.get_float4(theme_color::accent_medium));
			ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ctx_.current_theme.get_float4(theme_color::accent_light));
		}

		virtual void post_style() override
		{
			ImGui::PopStyleVar();
			ImGui::PopStyleColor(2);
		}
	};
}
