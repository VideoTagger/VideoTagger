#pragma once
#include <ui/popup.hpp>

namespace vt::ui
{
	struct script_progress_popup : public modal_popup
	{
	public:
		script_progress_popup(std::optional<bool*> open = std::nullopt);

	public:
		virtual void pre_style() override;
		virtual void post_style() override;

		virtual void on_render() override;
		virtual void post_render() override;
	};
}
