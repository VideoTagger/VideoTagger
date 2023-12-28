#pragma once
#include <vector>
#include <functional>
#include "core/project.hpp"

namespace vt::widgets
{
	class project_selector
	{
	public:
		project_selector() = default;
		project_selector(const std::vector<project>& projects);

		std::function<void(project&)> on_click_project;
		std::function<void()> on_project_list_update;
	private:
		std::vector<project> projects_;
		project temp_project;

	private:
		void render_project_creation_menu();
		void render_project_widget(size_t id, project& project);
	public:
		void sort();
		void remove(const project& project);
		void load_projects_file(const std::filesystem::path& filepath);
		void save_projects_file(const std::filesystem::path& filepath);
		void set_opened(bool value);
		void render();
	};
}
