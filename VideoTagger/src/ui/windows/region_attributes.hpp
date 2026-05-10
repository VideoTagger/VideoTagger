#pragma once
#include <ui/window.hpp>

namespace vt::ui::windows
{
	class region_attributes : public ui::window
	{
	public:
		region_attributes();

	public:
		virtual void on_render() override;
	};
}
