#pragma once
#include <string>
#include <vector>

#include <tags/tag_storage.hpp>
#include <ui/popup.hpp>

namespace vt::ui
{
	struct timeline_menu_popup : public popup
	{
	public:
		timeline_menu_popup(tag_storage* tags = nullptr);

	private:
		std::vector<std::string> displayed_tags_;
		tag_storage* tags_;

	public:
		virtual void on_render() override;

		void set_tag_storage(tag_storage* tags);
		void set_displayed_tags(const std::vector<std::string>& displayed_tags);

		const std::vector<std::string>& displayed_tags() const;
	};
}
