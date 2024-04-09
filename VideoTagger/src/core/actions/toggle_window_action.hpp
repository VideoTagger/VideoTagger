#pragma once
#include <core/keybind_action.hpp>

namespace vt
{
	struct toggle_window_action : public keybind_action
	{
	public:
		toggle_window_action(const char* settings_name, bool& value);

	private:
		const char* settings_name{};
		bool& value;

	public:
		virtual void invoke() const final override;
		virtual void render_properties() final override;
	};
}
