#include "pch.hpp"
#include "run_script_action.hpp"
#include <core/app_context.hpp>
#include <editor/run_script_command.hpp>

namespace vt
{
	run_script_action::run_script_action(const std::filesystem::path& script_path) : keybind_action(action_name), script_path_{ script_path } {}
	
	void run_script_action::invoke() const
	{
		if (!ctx_.current_project.has_value()) return;

		auto all_scripts = ctx_.scripts.all_children();
		auto it = std::find(all_scripts.begin(), all_scripts.end(), script_path_);
		if (it == all_scripts.end())
		{
			ctx_.console.add_entry(widgets::console::entry::flag_type::warn, fmt::format("Failed to run script '{}', since it doesn't exist", script_path_.string()), widgets::console::entry::source_info{ "VideoTagger", 0 });
			return;
		}
		ctx_.registry.execute<run_script_command>(script_path_.string());
	}

	void run_script_action::to_json(nlohmann::ordered_json& json) const
	{
		auto& script_name = json["script-path"];
		if (script_path_.empty())
		{
			script_name = nullptr;
		}
		else
		{
			script_name = script_path_;
		}
	}

	void run_script_action::from_json(const nlohmann::ordered_json& json)
	{
		if (json.contains("script-path"))
		{
			const auto& script_name = json.at("script-path");
			if (!script_name.is_null())
			{
				script_path_ = json.at("script-path").get<std::string>();
			}
		}
	}

	void run_script_action::render_properties()
	{
		ImGui::TableNextColumn();
		ImGui::Text("Script Path");
		ImGui::TableNextColumn();

		auto available_scripts = ctx_.scripts.all_children();

		int selected_script{};
		std::vector<std::string> script_names;
		int i = 0;
		for (const auto& script : available_scripts)
		{
			if (script == script_path_)
			{
				selected_script = i;
			}
			script_names.push_back(fmt::format("{} {}", icons::terminal, script.string()));
			++i;
		}

		std::vector<const char*> script_names_cstr;
		for (const auto& name : script_names)
		{
			script_names_cstr.push_back(name.c_str());
		}
		
		if (ImGui::Combo("##ScriptName", &selected_script, script_names_cstr.data(), static_cast<int>(script_names_cstr.size())))
		{
			script_path_ = available_scripts.at(static_cast<size_t>(selected_script));
		}
	}
}
