#pragma once
#include <string>

namespace vt
{
	struct project;
	struct keybind_action
	{
	public:
		keybind_action(const std::string& name) : name_{ name } {}

	private:
		std::string name_;

	public:
		virtual void invoke() const = 0;
		virtual void render_properties() = 0;
		const std::string& name() const;
	};
}
