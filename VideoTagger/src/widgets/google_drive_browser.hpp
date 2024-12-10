#pragma once
#include <string>
#include <vector>
#include <any>
#include <functional>
#include <optional>

namespace vt::widgets
{
	enum class google_drive_browser_item_type
	{
		folder,
		video,
		file,
	};

	struct google_drive_browser_item_data
	{
		google_drive_browser_item_data(std::string id, std::string name, google_drive_browser_item_type type)
			: id{ std::move(id) }, name{ std::move(name) }, type{ type }
		{}

		std::string id;
		std::string name;
		google_drive_browser_item_type type;
	};

	class google_drive_browser
	{
	public:
		static constexpr auto my_files_id = "my_files";
		static constexpr auto shared_files_id = "shared_files";

		ImVec2 tile_size = { 70.f, 70.f };

		google_drive_browser(std::string id);

		void clear();

		void push_folder(const std::string& folder_id, const std::string& folder_name);
		void pop_folder();
		void go_to_folder(size_t index);

		void set_item_context_menu(std::function<void(const std::string&, const google_drive_browser_item_data&)> item_context_menu);

		const google_drive_browser_item_data& current_folder() const;

		const std::optional<google_drive_browser_item_data>& selected_item() const;

		const std::vector<google_drive_browser_item_data>& current_path() const;

		bool update_items();
		const std::vector<google_drive_browser_item_data>& items() const;

		bool render();

	private:
		std::string id_;
		std::optional<google_drive_browser_item_data> selected_item_;
		std::vector<google_drive_browser_item_data> current_path_;
		std::vector<google_drive_browser_item_data> items_;
		std::function<void(const std::string& /*label*/, void* /*google_drive_browser_item_data*/)> item_context_menu_;
	};
}
