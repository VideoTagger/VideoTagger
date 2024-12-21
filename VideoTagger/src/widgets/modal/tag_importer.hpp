#pragma once
#include <vector>
#include <filesystem>
#include <tags/tag_storage.hpp>

namespace vt::widgets::modal
{
	struct imported_tag_data
	{
		tag tag;
		bool selected;
	};

	struct tag_importer
	{
		static constexpr std::string_view popup_id = "Import Tags";

		std::vector<imported_tag_data> imported_tags;
		std::filesystem::path tags_path;

		bool is_open() const;
		void open();

		bool load_tags();

		bool render(bool& is_open);
	};
}
