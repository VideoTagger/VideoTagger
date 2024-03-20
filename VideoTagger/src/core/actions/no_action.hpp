#pragma once
#include <core/keybind_action.hpp>

namespace vt
{
	struct no_action : public keybind_action
	{
		no_action();
		virtual void invoke() const final override;
		virtual void render_properties() final override;
	};
}
