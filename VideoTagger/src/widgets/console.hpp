#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <ui/window.hpp>

namespace vt::widgets
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
		bool clear_on_run_ = true;
		bool show_infos_ = true;
		bool show_warns_ = true;
		bool show_errors_ = true;

	public:
		virtual void on_render() override;
		virtual nlohmann::ordered_json serialize() const override;
		virtual void deserialize(const nlohmann::ordered_json& json) override;

		void on_run_script();

		void set_scripts_path(const std::filesystem::path& path);
		void add_entry(entry::flag_type flag, const std::string& message, const std::optional<entry::source_info>& info = std::nullopt);
		void clear();

		static std::string window_name();
	};
}
