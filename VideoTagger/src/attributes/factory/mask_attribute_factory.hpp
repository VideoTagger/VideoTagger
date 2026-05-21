#pragma once
#include <attributes/factory/shape_attribute_factory.hpp>
#include <attributes/tools/mask_tool.hpp>
#include <attributes/tools/wand_tool.hpp>

namespace vt
{
	class mask_attribute_factory : public shape_attribute_factory_mt<mask_shape, mask_tool, wand_tool>
	{
	public:
		mask_attribute_factory(const std::string& name, const std::string& icon);

	public:
		virtual std::unique_ptr<impl::attribute> new_attribute(const std::string& name) override;
	};
}
