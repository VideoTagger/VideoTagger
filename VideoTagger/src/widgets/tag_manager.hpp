#pragma once
#include <vector>
#include <optional>

#include <tags/tag_storage.hpp>

namespace vt::widgets
{
	//TODO: probably should be a class like every other widget

	enum class tag_manager_flags
	{
		none = 0,
		no_add = 1 << 0, // Don't display the add button
		no_remove = 1 << 1, // Don't display the remove button
	};

	inline constexpr tag_manager_flags operator|(const tag_manager_flags& lhs, const tag_manager_flags& rhs)
	{
		return static_cast<tag_manager_flags>(static_cast<uint64_t>(lhs) | static_cast<uint64_t>(rhs));
	}
	inline constexpr tag_manager_flags& operator|=(tag_manager_flags& lhs, const tag_manager_flags& rhs)
	{
		lhs = lhs | rhs;
		return lhs;
	}
	inline constexpr bool operator&(const tag_manager_flags& lhs, const tag_manager_flags& rhs)
	{
		return static_cast<bool>(static_cast<uint64_t>(lhs) & static_cast<uint64_t>(rhs));
	}
	
	struct tag_rename_data
	{
		bool processed = false;
		bool ready = false;
		std::string old_name;
		std::string new_name;
	};

	struct tag_delete_data
	{
		bool ready = false;
		std::string tag;
	};

	bool tag_manager(tag_storage& tags, std::optional<tag_rename_data>& tag_rename, std::optional<tag_delete_data>& tag_delete, bool& dirty_flag, tag_manager_flags flags = tag_manager_flags::none);
}
