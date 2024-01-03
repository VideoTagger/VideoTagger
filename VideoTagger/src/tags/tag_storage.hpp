#pragma once
#include <unordered_map>
#include <utility>

#include "tag.hpp"

namespace vt
{
	class tag_storage_const_iterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = tag;
		using difference_type = std::ptrdiff_t;
		using pointer = const tag*;
		using reference = const tag&;

		using unwrapped_it = std::unordered_map<std::string, tag>::const_iterator;

		tag_storage_const_iterator() = default;
		tag_storage_const_iterator(unwrapped_it it);

		tag_storage_const_iterator& operator++();
		tag_storage_const_iterator operator++(int);

		reference operator*() const;
		pointer operator->() const;

		bool operator==(const tag_storage_const_iterator& rhs) const;
		bool operator!=(const tag_storage_const_iterator& rhs) const;

		unwrapped_it unwrapped() const;

	private:
		unwrapped_it it;
	};

	class tag_storage_iterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = tag;
		using difference_type = std::ptrdiff_t;
		using pointer = tag*;
		using reference = tag&;
		using const_pointer = const tag*;
		using const_reference = const tag&;

		using unwrapped_it = std::unordered_map<std::string, tag>::iterator;

		tag_storage_iterator() = default;
		tag_storage_iterator(unwrapped_it it);

		tag_storage_iterator& operator++();
		tag_storage_iterator operator++(int);

		reference operator*();
		pointer operator->();
		const_reference operator*() const;
		const_pointer operator->() const;

		bool operator==(const tag_storage_iterator& rhs) const;
		bool operator!=(const tag_storage_iterator& rhs) const;

		unwrapped_it unwrapped() const;

	private:
		unwrapped_it it;
	};

	enum class tag_validate_result
	{
		ok,
		already_exists,
		invalid_name
	};

	//TODO: Maybe add tag renaming (would just make a copy with a new name and delete the old one)
	class tag_storage
	{
	public:
		using iterator = tag_storage_iterator;
		using const_iterator = tag_storage_const_iterator;

		static constexpr uint32_t default_tag_color = 0xffffff;

		std::pair<iterator, bool> insert(const std::string& name, uint32_t color);
		std::pair<iterator, bool> insert(const std::string& name);
		bool erase(const std::string& name);
		iterator erase(iterator it);
		iterator erase(const_iterator it);

		tag& at(const std::string& name);
		const tag& at(const std::string& name) const;
		tag& operator[](const std::string& name);
		const tag& operator[](const std::string& name) const;
		iterator find(const std::string& name);
		const_iterator find(const std::string& name) const;

		tag_validate_result validate_tag_name(const std::string& name) const;
		bool contains(const std::string& name) const;
		size_t size() const;
		bool empty() const;

		iterator begin();
		const_iterator begin() const;
		const_iterator cbegin() const;
		iterator end();
		const_iterator end() const;
		const_iterator cend() const;

	private:
		std::unordered_map<std::string, tag> tags_;
	};
}
