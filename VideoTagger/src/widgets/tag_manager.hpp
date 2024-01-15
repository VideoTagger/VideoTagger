#pragma once
#include <vector>

#include <tags/tag_storage.hpp>

namespace vt::widgets
{
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
	
	//struct tag_manager_state
	//{
	//	tag_storage* tags;
	//
	//};
	//
	bool tag_manager(tag_storage& tags, tag_storage::iterator& selected_entry, tag_manager_flags flags = tag_manager_flags::none);
}
