#pragma once
#include <string>
#include <type_traits>
#include <memory>
#include <typeinfo>
#include <unordered_map>
#include <attributes/impl/attribute_factory.hpp>
#include <core/debug.hpp>
#include <utils/json.hpp>

namespace vt
{
	struct attribute_registry_entry
	{
		std::unique_ptr<impl::attribute_factory> factory;
		uint32_t color;
	};

	class attribute_registry
	{
	private:
		std::unordered_map<std::string, attribute_registry_entry> registry_;

	public:
		std::unique_ptr<impl::attribute> derialize(const nlohmann::ordered_json& json)
		{
			if (!json.is_object())
			{
				debug::error("Invalid attribute JSON structure, expected an object at the root");
				return nullptr;
			}

			if (!json.contains("type") or !json.is_string())
			{
				debug::error("Invalid attribute JSON structure, missing or invalid 'type' field");
				return nullptr;
			}

			auto type = json["type"].get<std::string>();
			auto factory = get_factory(type);
			if (factory == nullptr)
			{
				debug::error("No attribute factory registered with name '{}'", type);
				return nullptr;
			}
			debug::log_src("attribute-registry", "Derializing attribute with type: '{}'", type);

		}

		std::vector<std::string> attribute_names() const
		{
			std::vector<std::string> result;
			for (auto& [name, _] : registry_)
			{
				result.push_back(name);
			}
			return result;
		}

		template<typename factory_type, typename... arguments, typename = std::enable_if_t<std::is_base_of_v<impl::attribute_factory, factory_type> and std::is_constructible_v<factory_type, arguments...>>>
		void new_factory(const std::string& name, uint32_t color, arguments&&... args)
		{
			debug::log_src("attribute-registry", "Registering attribute factory with name: '{}' and type: {}", name, typeid(factory_type).name());
			//TODO: Check for duplicate names
			registry_[name] = { std::make_unique<factory_type>(std::forward<arguments>(args)...), color };
		}

		virtual std::unique_ptr<impl::attribute> create(const std::string& name)
		{
			auto factory = get_factory(name);
			if (factory == nullptr) return nullptr;
			return factory->create();
		}

		virtual std::unique_ptr<impl::attribute_instance> instantiate(const std::string& name)
		{
			auto factory = get_factory(name);
			if (factory == nullptr) return nullptr;
			return factory->instantiate();
		}

	private:
		impl::attribute_factory* get_factory(const std::string& name) const
		{
			auto it = registry_.find(name);
			if (it == registry_.end())
			{
				debug::error("No attribute factory registered with name {}", name);
				return nullptr;
			}
			return it->second.factory.get();
		}
	};
}
