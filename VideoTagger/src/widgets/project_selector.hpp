#pragma once
#include <vector>
#include <functional>
#include "core/project.hpp"

namespace vt::widgets
{
	class project_selector
	{
	public:
		project_selector(const std::vector<project>& projects);

		std::function<void(const project&)> on_click_project;
	private:
		std::vector<project> projects_;

	private:
		void render_project_widget(size_t id, const project& project);
	public:
		void render();
	};
}
