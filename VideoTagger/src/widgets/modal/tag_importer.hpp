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
		std::vector<imported_tag_data> imported_tags;
		std::filesystem::path tags_path;

		void open();

		bool load_tags();

		bool render(bool& is_open);
	};
}
