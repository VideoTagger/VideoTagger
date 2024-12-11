#pragma once
#include "keybind_action.hpp"

namespace vt
{
	struct no_action : public keybind_action
	{
		static constexpr auto action_name = "None";

		no_action();
		virtual void invoke() const final override;
		virtual void to_json(nlohmann::ordered_json& json) const final override;
		virtual void from_json(const nlohmann::ordered_json& json) final override;
		virtual void render_properties() final override;
	};
}

