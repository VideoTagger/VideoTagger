#pragma once
#include <memory>
#include <string>
#include <array>
#include <stdexcept>

#include <attributes/impl/shape.hpp>
#include <attributes/core/shape_attribute.hpp>
#include <attributes/impl/attribute_factory.hpp>
#include <core/app_context.hpp>
#include <scripts/bindings/proxies.hpp>

namespace vt
{
	template<typename shape_type, typename = std::enable_if_t<std::is_base_of_v<impl::shape, shape_type>>>
	class shape_attribute_factory : public impl::attribute_factory
	{
	public:
		shape_attribute_factory(const std::string& name, const std::string& icon) : impl::attribute_factory{ name }, icon_{ icon } {}

	private:
		std::string icon_;

	public:
		const std::string& icon() const
		{
			return icon_;
		}

		virtual std::unique_ptr<impl::attribute> new_attribute(const std::string& name) override
		{
			auto ptr = std::make_unique<shape_attribute<shape_type>>(this, name);
			ptr->on_init();
			return ptr;
		}

		virtual std::unique_ptr<ui::toolbar_tool> new_tool(const tag& tag, const std::string& attribute_name) const
		{
			return new_tool_impl<shape_tool<shape_type>>(tag, attribute_name);
		}

		virtual ui::toolbar_tool_specification tool_specification() const
		{
			return ui::toolbar_tool_specification{ name(), icon(), utils::string::to_titlecase(name())};
		}

		template<typename tool_type, typename = std::enable_if_t<std::is_base_of_v<ui::toolbar_tool, tool_type>>>
		std::unique_ptr<ui::toolbar_tool> new_tool_impl(const tag& tag, const std::string& attribute_name) const
		{
			return std::make_unique<tool_type>(tag, attribute_name);
		}

		virtual void register_bindings() override
		{
			namespace py = pybind11;

			auto class_name = fmt::format("{}AttributeInstance", utils::string::to_titlecase(name()));
			auto class_region_name = fmt::format("{}Region", utils::string::to_titlecase(name()));
			ctx_.script_eng.bind([class_name, class_region_name](py::module_& mod)
			{
				auto attribute_submod = mod.attr("attributes");
				py::class_<bindings::vt_region<shape_type>>(attribute_submod, class_region_name.c_str())
				.def_property_readonly("keyframes", [](const bindings::vt_region<shape_type>& region) -> std::vector<std::tuple<timestamp, shape_type>>
				{
					std::vector<std::tuple<timestamp, shape_type>> result;
					for (const auto& [ts, shape] : region.instance.ptr->get_region(region.id))
					{
						result.push_back(std::make_tuple(ts, shape));
					}
					return result;
				})
				.def("get_keyframe", [](const bindings::vt_region<shape_type>& region, timestamp ts) -> std::optional<std::tuple<timestamp, shape_type>>
				{
					auto& reg = region.instance.ptr->get_region(region.id);
					auto it = reg.find_keyframe(ts);
					if (it == reg.end()) return std::nullopt;

					return std::make_tuple(it->first, it->second);
				}, py::arg("ts"))
				.def("insert_keyframe", [](const bindings::vt_region<shape_type>& region, timestamp ts, const shape_type& shape) -> void
				{
					auto& reg = region.instance.ptr->get_region(region.id);
					reg.insert_keyframe(ts, shape);
				})
				.def("remove_keyframe", [](const bindings::vt_region<shape_type>& region, timestamp ts) -> bool
				{
					auto& reg = region.instance.ptr->get_region(region.id);
					return reg.erase_keyframe(ts);
				})
				.def_property_readonly("interpolation_type", [](const bindings::vt_region<shape_type>& region) -> std::string
				{
					auto& reg = region.instance.ptr->get_region(region.id);
					return reg.interpolator().name();
				})
				.def("set_interpolation_type", [](const bindings::vt_region<shape_type>& region, const std::string& name) -> bool
				{
					auto& reg = region.instance.ptr->get_region(region.id);
					auto& interpolator_registry = ctx_.get_shape_interpolator_registry<shape_type>();
					auto interpolator = interpolator_registry.new_interpolator(name);
					if (interpolator == nullptr)
					{
						return false;
					}

					reg.set_interpolator(std::move(interpolator));
					return true;
				});

				py::class_<bindings::vt_shape_attribute_instance<shape_type>>(attribute_submod, class_name.c_str())
				.def(py::init([class_name](bindings::vt_attribute_instance& instance) -> bindings::vt_shape_attribute_instance<shape_type>
				{
					auto* ptr = instance.ptr->as<shape_attribute_instance<shape_type>>();
					if (ptr == nullptr)
					{
						throw std::runtime_error(fmt::format("Failed to cast AttributeInstance to {}, types are incompatible", class_name));
					}
					return bindings::vt_shape_attribute_instance<shape_type>{ instance.segment, ptr };
				}), py::arg("instance"))
				.def_property_readonly("regions", [](const bindings::vt_shape_attribute_instance<shape_type>& instance) -> std::vector<bindings::vt_region<shape_type>>
				{
					std::vector<bindings::vt_region<shape_type>> result;
					for (const auto& [id, _] : instance.ptr->regions())
					{
						result.push_back(bindings::vt_region<shape_type>{ id, instance });
					}
					return result;
				})
				.def("insert_region", [](bindings::vt_shape_attribute_instance<shape_type>& instance, const shape_type& shape, timestamp ts, const std::optional<std::string>& region_name = std::nullopt) -> bindings::vt_region<shape_type>
				{
					auto id = instance.ptr->insert_region(region_name, ts, shape);
					return bindings::vt_region<shape_type>{ id, instance };
				}, py::arg("shape"), py::arg("ts"), py::arg("name") = std::nullopt)
				.def("remove_region", [](bindings::vt_shape_attribute_instance<shape_type>& instance, const bindings::vt_region<shape_type>& region) -> bool
				{
					return instance.ptr->remove_region(region.id);
				}, py::arg("region"));
			});
		};
	};

	/**
	 * @brief Same as shape_attribute_factory but allows specifying a custom tool type. The tool type must be default constructible and derive from ui::toolbar_tool
	 * 
	 * @sa Use shape_attribute_factory if you don't need a custom tool type
	 */
	template<typename shape_type, typename tool_type, typename = std::enable_if_t<std::is_base_of_v<impl::shape, shape_type> and std::is_base_of_v<ui::toolbar_tool, tool_type>>>
	class shape_attribute_factory_ex : public shape_attribute_factory<shape_type>
	{
	public:
		shape_attribute_factory_ex(const std::string& name, const std::string& icon) : shape_attribute_factory<shape_type>{ name, icon } {}

	public:
		virtual std::unique_ptr<ui::toolbar_tool> new_tool(const tag& tag, const std::string& attribute_name) const override
		{
			return this->template new_tool_impl<tool_type>(tag, attribute_name);
		}
	};

	///@brief A shape attribute factory that supports multiple tools. The first tool type is specified as a template parameter, while the rest are specified as variadic template parameters. All tool types must derive from ui::toolbar_tool and be default constructible.
	template<typename shape_type, typename tool_type, typename... tool_types>
	class shape_attribute_factory_mt : public shape_attribute_factory<shape_type>
	{
	private:
		static constexpr size_t tool_count_ = sizeof...(tool_types) + 1;

	public:
		static_assert(std::is_base_of_v<impl::shape, shape_type> and std::is_base_of_v<ui::toolbar_tool, tool_type> and (std::is_base_of_v<ui::toolbar_tool, tool_types> and ...), "All tool types must derive from ui::toolbar_tool");
		shape_attribute_factory_mt(const std::string& name, const std::string& icon, const std::array<ui::toolbar_tool_specification, tool_count_>& tool_specs = {})
			: shape_attribute_factory<shape_type>{ name, icon }, tool_specs_{ tool_specs } {}

	private:
		std::array<ui::toolbar_tool_specification, tool_count_> tool_specs_;

	public:
		void set_tool_specifications(const std::array<ui::toolbar_tool_specification, tool_count_>& tool_specs)
		{
			tool_specs_ = tool_specs;
		}

		virtual ui::toolbar_tool_specification tool_specification() const override
		{
			return tool_specification(0);
		}

		virtual ui::toolbar_tool_specification tool_specification(size_t index) const
		{
			return tool_specs_.at(index);
		}

		virtual std::vector<std::unique_ptr<ui::toolbar_tool>> new_tools(const tag& tag, const std::string& attribute_name) const
		{
			std::vector<std::unique_ptr<ui::toolbar_tool>> tools;
			tools.reserve(tool_count_);
			tools.push_back(this->template new_tool_impl<tool_type>(tag, attribute_name));
			(tools.push_back(this->template new_tool_impl<tool_types>(tag, attribute_name)), ...);
			return tools;
		}

		virtual std::unique_ptr<ui::toolbar_tool> new_tool(const tag& tag, const std::string& attribute_name) const override
		{
			throw std::logic_error{ "This factory creates multiple tools, use new_tools() instead" };
		}

		static constexpr size_t tool_count()
		{
			return tool_count_;
		}
	};
}
