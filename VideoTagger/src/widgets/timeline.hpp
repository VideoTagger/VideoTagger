#pragma once
#include <imgui.h>
#include <string>

namespace vt::widgets
{
	struct timeline
	{
	private:
		float zoom_ = 1.f;

	public:
		void draw_marker();
		void render(bool& is_open);

		static std::string window_name();
	};
}
