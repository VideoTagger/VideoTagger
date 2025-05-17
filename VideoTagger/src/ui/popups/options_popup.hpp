#pragma once
#include <string>
#include <unordered_map>
#include <ui/popup.hpp>
#include <ui/widgets/text_input.hpp>

namespace vt::ui
{
	struct options_popup : public modal_popup
	{
	public:
		options_popup(std::optional<bool*> open = std::nullopt);

	private:
		std::string active_group_;
		std::string active_tab_;
		std::map<std::string, std::map<std::string, std::function<void()>>> groups_;

	public:
		virtual void on_display() override;
		virtual void on_render() override;

		virtual bool pre_render() override;

		void set_active_tab(const std::string& group, const std::string& name);
		std::function<void()>& operator()(const std::string& group, const std::string& name);
	};
}
