#pragma once
#include <vector>
#include <core/project.hpp>

namespace vt::widgets
{
	class project_selector
	{
	public:
		project_selector() = default;
		project_selector(const std::vector<project_info>& projects);

	private:
		std::string filter;
		bool path_from_name = true;
		std::vector<project_info> projects_;
		project_info temp_project;

	private:
		void render_project_creation_menu();
		void render_project_widget(size_t id, project_info& project);
	public:
		void sort();
		void remove(const project_info& project);
		void load_projects_file(const std::filesystem::path& filepath);
		void save_projects_file(const std::filesystem::path& filepath);
		void set_opened(bool value);
		void render();
	};
}
