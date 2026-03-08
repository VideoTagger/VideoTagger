#pragma once
#include <vector>
#include <optional>
#include <memory>

#include <tags/tag_storage.hpp>
#include <ui/window.hpp>
#include <ui/popups/delete_tag_popup.hpp>
#include <ui/popups/new_tag_popup.hpp>
#include <ui/popups/rename_tag_popup.hpp>
#include <ui/popups/add_tag_attribute_popup.hpp>

namespace vt::ui::windows
{
	class tag_manager : public ui::window
	{
	public:
		tag_manager();

	private:
		std::string filter_;
		std::string tag_name_;
		tag_storage::iterator color_ref_;

		std::unique_ptr<delete_tag_popup> delete_tag_popup_;
		std::unique_ptr<new_tag_popup> new_tag_popup_;
		std::unique_ptr<rename_tag_popup> rename_tag_popup_;
		std::unique_ptr<add_tag_attribute_popup> add_tag_attribute_popup_;

	public:
		virtual void on_display() override;
		virtual void on_render() override;

	};
}
