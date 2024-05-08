#pragma once
#include <functional>
#include <core/keybind_action.hpp>

namespace vt
{
	struct builtin_action : public keybind_action
	{
		static constexpr auto action_name = "Builtin Action";

	public:
		builtin_action(const std::function<void()>& action, const std::string& name = action_name);

	private:
		std::function<void()> action_;

	public:
		virtual void invoke() const final override;
		virtual void to_json(nlohmann::ordered_json& json) const final override;
		virtual void from_json(const nlohmann::ordered_json& json) final override;
		virtual void render_properties() final override;
	};
}
