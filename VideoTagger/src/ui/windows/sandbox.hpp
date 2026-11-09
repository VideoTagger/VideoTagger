#pragma once
#include <ui/window.hpp>
#include <ui/widgets/widget_list.hpp>

namespace vt::ui::windows
{
	struct sandbox : public window
	{
	public:
		sandbox();

	private:
		ui::widget_list widget_list_;

	public:
		void setup_widgets();

		virtual void on_render() override;
	};
}
