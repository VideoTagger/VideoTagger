#pragma once
#include <attributes/factory/shape_predictor_factory.hpp>
#include <attributes/factory/interpolated_shape_predictor_factory.hpp>
#include <attributes/impl/shape_predictor.hpp>
#include <attributes/impl/interpolated_shape_predictor.hpp>
#include <attributes/impl/shape_predictor_registry.hpp>

#include <core/debug.hpp>

namespace vt
{
	template<typename shape_t>
	class shape_predictor_registry : public impl::shape_predictor_registry
	{
	public:
		using shape_type = shape_t;

		shape_predictor_registry() = default;

	private:
		std::unordered_map<std::string, std::unique_ptr<shape_predictor_factory<shape_t>>> registry_;

	public:
		template<typename predictor_factory_type, typename... arguments>
		predictor_factory_type& new_factory(const std::string& name, arguments&&... args)
		{
			auto [it, inserted] = registry_.try_emplace(name, std::make_unique<predictor_factory_type>(name, std::forward<arguments>(args)...));
			auto& factory = dynamic_cast<predictor_factory_type&>(*it->second);
			if (!inserted)
			{
				debug::error("Predictor factory with name '{}' is already registered", name);
				return factory;
			}

			predictor_names_.push_back(name);

			if constexpr (std::is_base_of_v<interpolated_shape_predictor_factory<shape_t>, predictor_factory_type>)
			{
				interpolator_names_.push_back(name);

				if (!default_interpolator_name_.has_value())
				{
					default_interpolator_name_ = name;
				}
			}

			return factory;
		}

		std::unique_ptr<impl::shape_predictor<shape_t>> new_predictor(const std::string& name)
		{
			auto* factory = get_factory(name);
			if (factory == nullptr)
			{
				debug::error("No predictor factory registered with name '{}'", name);
				return nullptr;
			}

			return factory->new_shape_predictor();
		}

		std::unique_ptr<impl::interpolated_shape_predictor<shape_t>> new_interpolator(const std::string& name)
		{
			auto* factory = get_interpolator_factory(name);
			if (factory == nullptr)
			{
				debug::error("No predictor factory registered with name '{}'", name);
				return nullptr;
			}

			return factory->new_shape_interpolator();
		}

		std::unique_ptr<impl::interpolated_shape_predictor<shape_t>> new_default_interpolator()
		{
			if (!default_interpolator_name_.has_value()) return nullptr;

			return new_interpolator(*default_interpolator_name_);
		}

		shape_predictor_factory<shape_t>* get_factory(const std::string& name) const
		{
			auto it = registry_.find(name);
			if (it == registry_.end())
			{
				return nullptr;
			}
			return it->second.get();
		}

		interpolated_shape_predictor_factory<shape_t>* get_interpolator_factory(const std::string& name) const
		{
			auto it = registry_.find(name);
			if (it == registry_.end())
			{
				return nullptr;
			}

			auto* interpolator_factory = dynamic_cast<interpolated_shape_predictor_factory<shape_t>*>(it->second.get());
			if (interpolator_factory == nullptr)
			{
				debug::error("Predictor with name '{}' is not an interpolator", name);
				return nullptr;
			}

			return interpolator_factory;
		}
	};
}
