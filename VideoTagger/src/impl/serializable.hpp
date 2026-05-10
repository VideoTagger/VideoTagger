#pragma once
#include <utils/json.hpp>

namespace vt::impl
{
	struct serializable
	{
		virtual ~serializable() = default;
		
		[[nodiscard]] virtual nlohmann::ordered_json serialize() const { return {}; }
		virtual void deserialize(const nlohmann::ordered_json& json) {}
	};

	inline void to_json(nlohmann::ordered_json& json, const serializable& ref)
	{
		json = ref.serialize();
	}

	inline void from_json(const nlohmann::ordered_json& json, serializable& ref)
	{
		ref.deserialize(json);
	}
}
