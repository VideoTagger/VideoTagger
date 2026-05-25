#include "mask_attribute_factory.hpp"
#include <attributes/tools/mask_tool.hpp>
#include <attributes/tools/wand_tool.hpp>
#include <attributes/core/mask_attribute.hpp>

namespace vt
{
	mask_attribute_factory::mask_attribute_factory(const std::string& name, const std::string& icon) :
		shape_attribute_factory_mt<mask_shape, wand_tool, mask_tool>{ name, icon }
	{
		std::array<ui::toolbar_tool_specification, shape_attribute_factory_mt::tool_count()> tool_specs;
		tool_specs[0] = ui::toolbar_tool_specification("wand", icons::tool_wand, utils::string::to_titlecase("wand"));
		tool_specs[1] = ui::toolbar_tool_specification(this->name(), this->icon(), utils::string::to_titlecase(this->name()));
		set_tool_specifications(tool_specs);
	}

	std::unique_ptr<impl::attribute> mask_attribute_factory::new_attribute(const std::string& name)
	{
		auto ptr = std::make_unique<mask_attribute>(this, name);
		ptr->on_init();
		return ptr;
	}
}
