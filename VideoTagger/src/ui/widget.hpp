#pragma once
#include <string>
#include <ui/impl/renderable.hpp>

namespace vt::ui
{
	struct widget : public impl::renderable
	{
		bool render_with_label(const std::string& label, bool sameline = false);
		bool render_disabled(bool disabled = true);
		void sameline() const;

		bool is_enabled() const;
		bool is_disabled() const;
	};
}
