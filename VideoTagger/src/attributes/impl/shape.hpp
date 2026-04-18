#pragma once
#include <events/event_source.hpp>
#include <impl/serializable.hpp>

namespace vt::impl
{
	struct shape_config
	{
		bool interpolate{};
	};

	class shape : public serializable
	{
	public:
		shape() = default;
		virtual ~shape() = default;

	private:
		shape_config cfg_;

	public:
		virtual void set_target(event_source source) = 0;

		//[[nodiscard]] virtual nlohmann::ordered_json serialize() const = 0;
		//virtual void deserialize(const nlohmann::ordered_json& json) = 0;
	};
}
