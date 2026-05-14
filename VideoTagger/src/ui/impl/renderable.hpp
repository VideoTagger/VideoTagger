#pragma once

namespace vt::ui::impl
{
	struct renderable
	{
		virtual bool render() = 0;
	};
}
