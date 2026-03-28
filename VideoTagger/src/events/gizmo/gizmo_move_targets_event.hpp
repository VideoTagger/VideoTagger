#pragma once
#include <utils/vec.hpp>
#include "gizmo_targets_event.hpp"

namespace vt
{
	enum class gizmo_move_type
	{
		offset,
		absolute,
	};

	struct gizmo_move_targets_event : public gizmo_targets_event
	{
	public:
		gizmo_move_targets_event(const std::vector<utils::vec2<uint32_t>*>& targets, const utils::vec2<uint32_t>& value, gizmo_move_type type = gizmo_move_type::offset) : gizmo_targets_event{ targets }, value_{ value }, type_{ type } {}

	private:
		utils::vec2<uint32_t> value_;
		gizmo_move_type type_;

	public:
		[[nodiscard]] const utils::vec2<uint32_t>& value() const
		{
			return value_;
		}

		[[nodiscard]] gizmo_move_type move_type() const
		{
			return type_;
		}
	};
}
