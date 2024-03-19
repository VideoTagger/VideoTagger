#pragma once
#include <functional>
#include <core/input.hpp>

namespace vt
{
	struct builtin_action : public keybind_action
	{
	public:
		builtin_action(const std::function<void()>& action, const std::string& name = "Builtin Action");

	private:
		std::function<void()> action_;

	public:
		virtual void invoke() const final override;
		virtual void render_properties() final override;
	};
}
