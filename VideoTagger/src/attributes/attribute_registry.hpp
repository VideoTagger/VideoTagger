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
	struct attribute_specification
	{
		uint32_t color;
	};

	struct attribute_registry_entry
	{
		std::unique_ptr<impl::attribute_factory> factory;
		attribute_specification spec;
	};

	class attribute_registry
	{
	private:
		std::unordered_map<std::string, attribute_registry_entry> registry_;

	public:
		std::unique_ptr<impl::attribute> deserialize_attribute(const nlohmann::ordered_json& json)
		{
			if (!json.is_object())
			{
				debug::error("Invalid attribute JSON structure, expected an object at the root");
				return nullptr;
			}

			if (!json.contains("type") or !json["type"].is_string())
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
			debug::log_src("attribute-registry", "Deserializing attribute with type: '{}'", type);
			if (!json.contains("name") or !json["name"].is_string())
			{
				debug::error("Invalid attribute JSON structure, missing or invalid 'name' field");
			}
			auto name = json["name"].get<std::string>();
			return factory->new_attribute(name);
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

		std::vector<std::string> title_attribute_names() const
		{
			std::vector<std::string> result;
			for (auto& [name, _] : registry_)
			{
				result.push_back(utils::string::to_titlecase(name));
			}
			return result;
		}

		template<typename factory_type, typename... arguments, typename = std::enable_if_t<std::is_base_of_v<impl::attribute_factory, factory_type> and std::is_constructible_v<factory_type, const std::string&, arguments...>>>
		factory_type* new_factory(const std::string& name, uint32_t color, arguments&&... args)
		{
			debug::log_src("attribute-registry", "Registering attribute factory with name: '{}' and type: {}", name, typeid(factory_type).name());
			//TODO: Check for duplicate names
			auto factory = std::make_unique<factory_type>(name, std::forward<arguments>(args)...);
			auto factory_ptr = factory.get();
			attribute_specification spec{ color };
			registry_[name] = { std::move(factory), spec };
			return factory_ptr;
		}

		const attribute_specification* get_attr_spec(const std::string& name) const
		{
			auto it = registry_.find(name);
			if (it == registry_.end())
			{
				debug::error("No attribute factory registered with name '{}'", name);
				return nullptr;
			}
			return &it->second.spec;
		}

		virtual std::unique_ptr<impl::attribute> new_attribute(const std::string& type_name, const std::string& name)
		{
			auto factory = get_factory(type_name);
			if (factory == nullptr) return nullptr;
			return factory->new_attribute(name);
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
