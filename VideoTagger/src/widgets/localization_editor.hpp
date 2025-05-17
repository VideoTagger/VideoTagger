#pragma once
#include <string>
#include <vector>
#include <memory>
#include <core/localization/lang_pack.hpp>
#include <ui/popups/new_language_popup.hpp>
#include <ui/popups/remove_language_popup.hpp>

namespace vt::widgets
{
	struct localization_editor
	{
	public:
		localization_editor();

	private:
		std::unique_ptr<ui::new_language_popup> new_lang_popup_;
		std::unique_ptr<ui::remove_language_popup> remove_lang_popup_;

	public:
		void render(bool& is_open, std::vector<std::shared_ptr<lang_pack>>& langs);

		static std::string window_name();
	private:
		std::vector<std::string> keys(const std::vector<std::shared_ptr<lang_pack>>& langs) const;
	};
}
