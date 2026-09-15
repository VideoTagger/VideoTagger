#pragma once
#include <ui/window.hpp>

namespace vt::ui::windows
{
	class region_properties : public ui::window
	{
	public:
		region_properties();

	public:
		virtual void on_render() override;
	};
}
