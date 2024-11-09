#pragma once
#include <map>
#include <utility>
#include <string>
#include <iterator>
#include <utils/json.hpp>
#include <utils/color.hpp>

#include "tag.hpp"

namespace vt
{
	enum class tag_validate_result
	{
		ok,
		already_exists,
		invalid_name,
		too_long
	};

	struct tag_rename_result;

	class tag_storage_iterator;
	class tag_storage_const_iterator;

	class tag_storage
	{
	public:
		using iterator = tag_storage_iterator;
		using const_iterator = tag_storage_const_iterator;

		using container = std::map<std::string, tag>;

		static constexpr uint32_t default_tag_color = 0xFFFFFFFF;
		static constexpr size_t max_tag_name_length = 20; // not equivalent to the max size in bytes
		static constexpr std::string_view forbidden_characters = "\t\n\r\a\b\v\f\"\'\\/<>|:?*";
		static constexpr std::string_view forbidden_edge_characters = " "; // tag name can't contain these characters at the very start or end

		std::pair<iterator, bool> insert(const tag& tag);
		std::pair<iterator, bool> insert(const std::string& name, uint32_t color);
		std::pair<iterator, bool> insert(const std::string& name);
		bool erase(const std::string& name);
		iterator erase(iterator it);
		iterator erase(const_iterator it);
		void clear();

		// returns:
		//	if current_name doesn't exist: { end, false }
		//	if new_name already exists: { iterator-to-new_name, false }
		//	otherwise: { iterator-to-inserted, true }
		tag_rename_result rename(const std::string& current_name, const std::string& new_name);

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
		container tags_;
	};

	class tag_storage_const_iterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = tag;
		using difference_type = std::ptrdiff_t;
		using pointer = const tag*;
		using reference = const tag&;

		using unwrapped_it = tag_storage::container::const_iterator;

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

		using unwrapped_it = tag_storage::container::iterator;

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

	struct tag_rename_result
	{
		tag_storage::iterator iterator;
		bool inserted;
		tag_validate_result validation_result;
	};

	inline void to_json(nlohmann::ordered_json& json, const tag_storage& ts)
	{
		json = nlohmann::json::array();
		for (auto& tag : ts)
		{
			nlohmann::ordered_json json_tag_data;
			json_tag_data["name"] = tag.name;
			json_tag_data["color"] = utils::color::to_string(tag.color);
			json.push_back(json_tag_data);
		}
	}

	inline void from_json(const nlohmann::ordered_json& json, tag_storage& ts)
	{
		for (const auto& tag_data : json)
		{
			if (!tag_data.contains("name") or !tag_data.contains("color")) continue;

			auto [tag_it, success] = ts.insert(tag_data["name"]);
			auto col_str = tag_data["color"].get<std::string>();
			uint32_t color{};
			if (utils::color::parse_string(col_str, color))
			{
				tag_it->color = color;
			}
		}
	}
}
