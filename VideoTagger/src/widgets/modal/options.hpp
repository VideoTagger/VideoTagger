#pragma once
#include <string>
#include <functional>
#include <map>

namespace vt::widgets::modal
{
	class options
	{
	public:
		options();

	private:
		std::string active_group;
		std::string active_tab;
		std::map<std::string, std::map<std::string, std::function<void()>>> groups;

	public:
		bool render(bool* is_open);
		void set_active_tab(const std::string& group, const std::string& name);
		std::function<void()>& operator()(const std::string& group, const std::string& name);
	};
}
