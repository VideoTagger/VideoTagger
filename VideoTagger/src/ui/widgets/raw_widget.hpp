#pragma once
#include <ui/widget.hpp>
#include <functional>

namespace vt::ui
{
	struct raw_widget : public widget
	{
	public:
		raw_widget(const std::function<bool()>& render_callback);

	private:
		std::function<bool()> render_callback_;

	public:
		virtual bool render() override;
	};
}
