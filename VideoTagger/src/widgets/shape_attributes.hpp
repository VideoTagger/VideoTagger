#pragma once
#include <ui/window.hpp>

namespace vt::widgets
{
	class shape_attributes : public ui::window
	{
	public:
		shape_attributes();

	public:
		virtual void on_render() override;
	};
}
