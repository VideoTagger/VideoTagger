#pragma once

namespace vt::impl
{
	struct resettable
	{
		virtual ~resettable() = default;
		virtual void reset() = 0;
	};
}
