#pragma once
#include <ui/window.hpp>

namespace vt::ui::windows
{
	class region_list : public ui::window
	{
	public:
		region_list();

	public:
		virtual void on_render() override;
	};
}
