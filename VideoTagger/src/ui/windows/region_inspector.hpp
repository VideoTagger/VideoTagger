#pragma once
#include <ui/window.hpp>

namespace vt::ui::windows
{
	class region_inspector : public ui::window
	{
	public:
		region_inspector();

	public:
		virtual void on_render() override;
	};
}
