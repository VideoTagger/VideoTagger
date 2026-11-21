#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <unordered_map>
#include <ui/window.hpp>

namespace vt::ui::windows
{
	struct console : public ui::window
	{
		struct entry
		{
			enum class flag_type
			{
				info,
				warn,
				error
			} flag;
			std::string message;

			struct source_info
			{
				std::filesystem::path path;
				int64_t line{};
			};
			std::optional<source_info> info;
		};

		console();

	private:
		std::vector<entry> entries_;
		std::string filter;
		std::filesystem::path scripts_path_;
		std::unordered_map<entry::flag_type, bool> show_flags_;
		bool clear_on_run_ = true;

	public:
		virtual void on_display() override;
		virtual void on_render() override;
		virtual nlohmann::ordered_json serialize() const override;
		virtual void deserialize(const nlohmann::ordered_json& json) override;

		void on_run_script();

		void set_scripts_path(const std::filesystem::path& path);
		void add_entry(entry::flag_type flag, const std::string& message, const std::optional<entry::source_info>& info = std::nullopt);
		void clear();
	};
}
