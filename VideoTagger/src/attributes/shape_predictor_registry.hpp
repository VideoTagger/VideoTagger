#pragma once
#include <attributes/factory/shape_predictor_factory.hpp>
#include <attributes/impl/shape_predictor.hpp>
#include <attributes/impl/interpolated_shape_predictor.hpp>

#include <core/debug.hpp>

namespace vt
{
	template<typename shape_t>
	class shape_predictor_registry
	{
	public:
		using shape_type = shape_t;

		shape_predictor_registry() = default;

	private:
		std::unordered_map<std::string, std::unique_ptr<shape_predictor_factory<shape_t>>> registry_;

	public:
		std::vector<std::string> predictor_names() const
		{
			std::vector<std::string> result;
			for (auto& [name, _] : registry_)
			{
				result.push_back(name);
			}
			return result;
		}

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
			auto predictor = new_predictor(name);
			auto* interpolator_ptr = dynamic_cast<impl::interpolated_shape_predictor<shape_t>*>(predictor.get());
			if (interpolator_ptr == nullptr) return nullptr;

			predictor.release();
			return std::unique_ptr<impl::interpolated_shape_predictor<shape_t>>{ interpolator_ptr };
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
	};
}
