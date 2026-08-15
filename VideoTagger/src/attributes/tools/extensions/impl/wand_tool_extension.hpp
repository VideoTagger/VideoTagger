#pragma once
#include <ui/toolbar/toolbar_tool_extension.hpp>
#include <attributes/impl/with_shape_data.hpp>
#include <attributes/shapes/mask_shape.hpp>
#include <string>

namespace vt::ui::impl
{
	struct wand_tool_extension : public toolbar_tool_extension, public vt::impl::with_shape_data<mask_shape>
	{
	public:
		wand_tool_extension(const std::string& name);

	private:
		std::string name_;

	public:
		const std::string& name() const;

		virtual void prepare_for_use();
		virtual bool is_ready();
	};
}
