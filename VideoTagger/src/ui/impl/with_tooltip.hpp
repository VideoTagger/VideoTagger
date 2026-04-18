#pragma once
#include <string>

namespace vt::ui::impl
{
	class with_tooltip
	{
	public:
		with_tooltip() = default;
		with_tooltip(const std::string& tooltip) : tooltip_{ tooltip } {}

	private:
		std::string tooltip_;

	public:
		const std::string& tooltip() const
		{
			return tooltip_;
		}

		void set_tooltip(const std::string& tooltip)
		{
			tooltip_ = tooltip;
		}
	};
}
