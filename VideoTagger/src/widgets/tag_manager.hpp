#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <limits>
#include <functional>

#include <tags/tag_storage.hpp>

namespace vt::widgets
{
	//struct tag_data
	//{
	//	std::string tag;
	//	uint32_t color{};
	//};
	//
	//class tag_manager_state
	//{
	//public:
	//	static constexpr size_t npos = std::numeric_limits<size_t>::max();
	//	std::function<void(tag_manager_state&, size_t)> remove_callback;
	//	std::function<void(tag_manager_state&, size_t)> add_callback;
	//
	//	bool add(tag_data data);
	//	void remove(size_t index);
	//
	//	size_t find(std::string_view tag) const;
	//	bool contains(std::string_view tag) const;
	//	const tag_data& get(size_t index) const;
	//	const std::vector<tag_data>& tags() const;
	//	size_t size() const;
	//	bool is_sorted() const;
	//
	//private:
	//	std::vector<tag_data> tags_;
	//	bool is_sorted_{};
	//};

	bool tag_manager(tag_storage& tags, tag_storage::iterator& selected_entry);
}
