#pragma once
#include <impl/serializable.hpp>

namespace vt::impl
{
	struct attribute : public serializable
	{

	};

	inline void to_json(nlohmann::ordered_json& json, const attribute& attr)
	{
		json = attr.serialize();
	}

	inline void from_json(const nlohmann::ordered_json& json, attribute& attr)
	{
		attr.deserialize(json);
	}
}
