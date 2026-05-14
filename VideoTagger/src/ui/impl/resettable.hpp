#pragma once

namespace vt::ui::impl
{
	struct resettable
	{
		virtual void reset() = 0;
	};
}
