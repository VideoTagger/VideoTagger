#pragma once
#include <string>
#include <optional>
#include "video_timeline.hpp"

namespace vt::widgets
{
	class shape_attributes
	{
	public:
		shape_attributes() = default;

	public:
		void render(std::optional<selected_segment_data>& selected_segment, bool& is_open);

		static std::string window_name();
	};
}
