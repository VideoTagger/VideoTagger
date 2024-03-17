#pragma once
#include <string>

namespace vt
{
	struct keybind_action
	{
	public:
		keybind_action(const std::string& name = "None") : name_{ name } {}

	private:
		std::string name_;

	public:
		virtual void invoke() const = 0;
		const std::string& name() const;
	};
}
