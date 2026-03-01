#pragma once
#include <utils/json.hpp>

namespace vt::impl
{
	struct serializable
	{
		virtual ~serializable() = default;
		
		[[nodiscard]] virtual nlohmann::ordered_json serialize() const { return {}; };
		virtual void deserialize(const nlohmann::ordered_json& json) {};
	};
}
