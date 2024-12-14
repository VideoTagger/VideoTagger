#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace vt::widgets
{
	struct console
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

		console() = default;

	private:
		std::vector<entry> entries_;
		std::string filter;
		bool show_infos = true;
		bool show_warns = true;
		bool show_errors = true;

	public:
		void render(bool& is_open, bool& clear_on_run, const std::filesystem::path& scripts_path);
		void add_entry(entry::flag_type flag, const std::string& message, const std::optional<entry::source_info>& info = std::nullopt);
		void clear();

		static std::string window_name();
	};
}
