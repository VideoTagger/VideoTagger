#pragma once
#include "app_event.hpp"

namespace vt
{
	struct request_save_settings_event : public app_event
	{
		request_save_settings_event() = default;
	};
}
