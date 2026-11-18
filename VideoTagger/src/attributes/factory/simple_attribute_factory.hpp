#pragma once
#include <attributes/impl/attribute.hpp>
#include <attributes/impl/attribute_factory.hpp>
#include <attributes/core/simple_attribute.hpp>
#include <scripts/bindings/proxies.hpp>

namespace vt
{
	template<typename attribute_type>
	class simple_attribute_factory : public impl::attribute_factory
	{
	public:
		simple_attribute_factory(const std::string& name) : impl::attribute_factory{ name } {}

	public:
		virtual std::unique_ptr<impl::attribute> new_attribute(const std::string& name) override
		{
			auto ptr = std::make_unique<simple_attribute<attribute_type>>(this, name);
			ptr->on_init();
			return ptr;
		}

		virtual void register_bindings() override
		{
			auto class_name = fmt::format("{}AttributeInstance", utils::string::to_titlecase(name()));
			ctx_.script_eng.bind([class_name](pybind11::module_& mod)
			{
				auto attribute_submod = mod.attr("attributes");

				pybind11::class_<bindings::vt_simple_attribute_instance<attribute_type>>(attribute_submod, class_name.c_str())
				.def(pybind11::init([class_name](bindings::vt_attribute_instance& instance) -> bindings::vt_simple_attribute_instance<attribute_type>
				{
					auto* ptr = instance.ptr->as<simple_attribute_instance<attribute_type>>();
					if (ptr == nullptr)
					{
						throw std::runtime_error(fmt::format("Failed to cast AttributeInstance to {}, types are incompatible", class_name));
					}
					return bindings::vt_simple_attribute_instance<attribute_type>{ instance.segment, ptr };
				}), pybind11::arg("instance"))
				.def_property("value", [](const bindings::vt_simple_attribute_instance<attribute_type>& instance)
				{
					return instance.ptr->value();
				},
				[](bindings::vt_simple_attribute_instance<attribute_type>& instance, const attribute_type& value)
				{
					instance.ptr->set_value(value);
				});
			});
		};
	};
}
