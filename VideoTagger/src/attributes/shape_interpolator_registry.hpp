#pragma once
#include <attributes/factory/shape_interpolator_factory.hpp>
#include <attributes/impl/shape_interpolator.hpp>
#include <attributes/impl/shape_interpolator_registry.hpp>

#include <core/debug.hpp>

namespace vt
{
	template<typename shape_t>
	class shape_interpolator_registry : public impl::shape_interpolator_registry
	{
	public:
		using shape_type = shape_t;

		shape_interpolator_registry() = default;

	private:
		std::unordered_map<std::string, std::unique_ptr<shape_interpolator_factory<shape_t>>> registry_;

	public:
		template<typename interpolator_factory_type, typename... arguments>
		interpolator_factory_type& new_factory(const std::string& name, arguments&&... args)
		{
			auto [it, inserted] = registry_.try_emplace(name, std::make_unique<interpolator_factory_type>(name, std::forward<arguments>(args)...));
			auto& factory = dynamic_cast<interpolator_factory_type&>(*it->second);
			if (!inserted)
			{
				debug::error("Interpolator factory with name '{}' is already registered", name);
				return factory;
			}

			interpolator_names_.push_back(name);

			if (!default_name_.has_value())
			{
				default_name_ = name;
			}

			return factory;
		}

		std::unique_ptr<impl::shape_interpolator<shape_t>> new_interpolator(const std::string& name)
		{
			auto* factory = get_factory(name);
			if (factory == nullptr)
			{
				debug::error("No interpolator factory registered with name '{}'", name);
				return nullptr;
			}

			return factory->new_shape_interpolator();
		}

		std::unique_ptr<impl::shape_interpolator<shape_t>> new_default_interpolator()
		{
			if (!default_name_.has_value()) return nullptr;

			return new_interpolator(*default_name_);
		}

		shape_interpolator_factory<shape_t>* get_factory(const std::string& name) const
		{
			auto it = registry_.find(name);
			if (it == registry_.end())
			{
				return nullptr;
			}
			return it->second.get();
		}
	};
}
