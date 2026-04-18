#include "attribute.hpp"
#include <attributes/impl/attribute_factory.hpp>
#include <core/debug.hpp>

namespace vt::impl
{
	attribute::attribute(attribute_factory* factory, const std::string& name) : factory_{ factory }, name_ { name } {}

	void attribute::set_name(const std::string& name)
	{
		name_ = name;
	}

	std::string& attribute::name()
	{
		return name_;
	}

	const std::string& attribute::name() const
	{
		return name_;
	}

    const std::string& attribute::type_name() const
    {
		return factory()->name();
    }

	attribute_factory* attribute::factory() const
	{
		return factory_;
	}

	std::unique_ptr<impl::attribute_instance> attribute::deserialize_instance(const nlohmann::ordered_json& json)
	{
		auto instance = instantiate();
		instance->deserialize(json);
		return instance;
	}

	[[nodiscard]] nlohmann::ordered_json attribute::serialize() const
	{
		nlohmann::ordered_json result;
		result["name"] = name();
		result["type"] = type_name();
		return result;
	};

	void attribute::deserialize(const nlohmann::ordered_json& json)
	{
		if (json.contains("name") and json["name"].is_string())
		{
			name_ = json["name"].get<std::string>();
		}

		if (json.contains("type") and json["type"].is_string())
		{
			if (json["type"].get<std::string>() != type_name())
			{
				debug::error("Attribute type mismatch during deserialization. Expected '{}', got '{}'", type_name(), json["type"].get<std::string>());
			}
		}
	};
}
