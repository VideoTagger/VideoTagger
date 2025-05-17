#pragma once
#include <string>
#include <ui/widget.hpp>

namespace vt::ui
{
	struct label_spacer : public widget
	{
	public:
		label_spacer(const std::string& label);

	private:
		std::string label_;

	public:
		virtual bool render() override;
	};
}
