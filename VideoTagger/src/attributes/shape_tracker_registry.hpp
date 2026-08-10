#pragma once
#include <attributes/factory/shape_tracker_factory.hpp>
#include <attributes/impl/shape_tracker.hpp>
#include <attributes/impl/shape_tracker_registry.hpp>

#include <core/debug.hpp>

namespace vt
{
	template<typename shape_t>
	class shape_tracker_registry : public impl::shape_tracker_registry
	{
	public:
		using shape_type = shape_t;

		shape_tracker_registry() = default;

	private:
		std::unordered_map<std::string, std::unique_ptr<shape_tracker_factory<shape_t>>> registry_;

	public:
		template<typename tracker_factory_type, typename... arguments>
		tracker_factory_type& new_factory(const std::string& name, arguments&&... args)
		{
			auto [it, inserted] = registry_.try_emplace(name, std::make_unique<tracker_factory_type>(name, std::forward<arguments>(args)...));
			auto& factory = dynamic_cast<tracker_factory_type&>(*it->second);
			if (!inserted)
			{
				debug::error("Tracker factory with name '{}' is already registered", name);
				return factory;
			}

			tracker_names_.push_back(name);

			if (!default_name_.has_value())
			{
				default_name_ = name;
			}

			return factory;
		}

		std::unique_ptr<impl::shape_tracker<shape_t>> new_tracker(const std::string& name)
		{
			auto* factory = get_factory(name);
			if (factory == nullptr)
			{
				debug::error("No tracker factory registered with name '{}'", name);
				return nullptr;
			}

			return factory->new_shape_tracker();
		}

		std::unique_ptr<impl::shape_tracker<shape_t>> new_default_tracker()
		{
			if (!default_name_.has_value()) return nullptr;

			return new_tracker(*default_name_);
		}

		shape_tracker_factory<shape_t>* get_factory(const std::string& name) const
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
