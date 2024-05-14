#include "pch.hpp"
#include "tag_storage.hpp"

namespace vt
{
	std::pair<tag_storage::iterator, bool> tag_storage::insert(const std::string& name, uint32_t color)
	{
		if (name.empty())
		{
			return { end(), false };
		}

		auto [it, result] = tags_.try_emplace(name, name, color);
		return { iterator(it), result };
	}

	std::pair<tag_storage::iterator, bool> tag_storage::insert(const std::string& name)
	{
		return insert(name, default_tag_color);
	}

	bool tag_storage::erase(const std::string& name)
	{
		return tags_.erase(name) != 0;
	}

	tag_storage::iterator tag_storage::erase(iterator it)
	{
		return iterator{ tags_.erase(it.unwrapped()) };
	}

	tag_storage::iterator tag_storage::erase(const_iterator it)
	{
		return iterator{ tags_.erase(it.unwrapped()) };
	}

	tag_rename_result tag_storage::rename(const std::string& current_name, const std::string& new_name)
	{
		auto current_it = tags_.find(current_name);
		if (current_it == tags_.end())
		{
			return { tags_.end(), false, tag_validate_result{} };
		}

		//if (auto it = tags_.find(new_name); it != tags_.end())
		//{
		//	return { it, false };
		//}

		if (auto validate_result = validate_tag_name(new_name); validate_result != tag_validate_result::ok)
		{
			return { tags_.end(), false, validate_result };
		}
		
		auto node_handle = tags_.extract(current_name);
		node_handle.key() = new_name;
		node_handle.mapped().name = new_name;
		auto insert_return = tags_.insert(std::move(node_handle));

		return { iterator{ insert_return.position }, true, tag_validate_result::ok };
	}

	tag& tag_storage::at(const std::string& name)
	{
		return tags_.at(name);
	}

	const tag& tag_storage::at(const std::string& name) const
	{
		return tags_.at(name);
	}

	tag& tag_storage::operator[](const std::string& name)
	{
		return at(name);
	}

	const tag& tag_storage::operator[](const std::string& name) const
	{
		return at(name);
	}

	tag_storage::iterator tag_storage::find(const std::string& name)
	{
		return iterator(tags_.find(name));
	}

	tag_storage::const_iterator tag_storage::find(const std::string& name) const
	{
		return const_iterator(tags_.find(name));
	}

	tag_validate_result tag_storage::validate_tag_name(const std::string& name) const
	{
		if (name.empty())
		{
			return tag_validate_result::invalid_name;
		}

		if (contains(name))
		{
			return tag_validate_result::already_exists;
		}

		auto string_length = utf8nlen(name.c_str(), name.size());
		if (string_length > max_tag_name_length)
		{
			return tag_validate_result::too_long;
		}
		
		if (auto pos = name.find_first_of(forbidden_edge_characters); pos == 0 or pos == name.size() - 1)
		{
			return tag_validate_result::invalid_name;
		}

		if (name.find_first_of(forbidden_characters) != std::string::npos)
		{
			return tag_validate_result::invalid_name;
		}

		return tag_validate_result::ok;
	}

	bool tag_storage::contains(const std::string& name) const
	{
		return tags_.find(name) != tags_.end();
	}

	size_t tag_storage::size() const
	{
		return tags_.size();
	}

	bool tag_storage::empty() const
	{
		return tags_.empty();
	}

	tag_storage::iterator tag_storage::begin()
	{
		return iterator(tags_.begin());
	}

	tag_storage::const_iterator tag_storage::begin() const
	{
		return const_iterator(tags_.begin());
	}

	tag_storage::const_iterator tag_storage::cbegin() const
	{
		return begin();
	}

	tag_storage::iterator tag_storage::end()
	{
		return iterator(tags_.end());
	}

	tag_storage::const_iterator tag_storage::end() const
	{
		return const_iterator(tags_.end());
	}

	tag_storage::const_iterator tag_storage::cend() const
	{
		return end();
	}

	tag_storage_const_iterator::tag_storage_const_iterator(unwrapped_it it)
		: it{ it }
	{
	}

	tag_storage_const_iterator& tag_storage_const_iterator::operator++()
	{
		++it;
		return *this;
	}

	tag_storage_const_iterator tag_storage_const_iterator::operator++(int)
	{
		auto prev_it = *this;
		++it;
		return prev_it;
	}

	tag_storage_const_iterator::reference tag_storage_const_iterator::operator*() const
	{
		auto& [k, v] = *it;
		return v;
	}

	tag_storage_const_iterator::pointer tag_storage_const_iterator::operator->() const
	{
		auto& [k, v] = *it;
		return &v;
	}

	bool tag_storage_const_iterator::operator==(const tag_storage_const_iterator& rhs) const
	{
		return it == rhs.it;
	}

	bool tag_storage_const_iterator::operator!=(const tag_storage_const_iterator& rhs) const
	{
		return it != rhs.it;
	}

	tag_storage_const_iterator::unwrapped_it tag_storage_const_iterator::unwrapped() const
	{
		return it;
	}

	tag_storage_iterator::tag_storage_iterator(unwrapped_it it)
		: it{ it }
	{
	}

	tag_storage_iterator& tag_storage_iterator::operator++()
	{
		++it;
		return *this;
	}

	tag_storage_iterator tag_storage_iterator::operator++(int)
	{
		auto prev_it = *this;
		++it;
		return prev_it;
	}

	tag_storage_iterator::reference tag_storage_iterator::operator*()
	{
		auto& [k, v] = *it;
		return v;
	}

	tag_storage_iterator::pointer tag_storage_iterator::operator->()
	{
		auto& [k, v] = *it;
		return &v;
	}

	tag_storage_iterator::const_reference tag_storage_iterator::operator*() const
	{
		auto& [k, v] = *it;
		return v;
	}

	tag_storage_iterator::const_pointer tag_storage_iterator::operator->() const
	{
		auto& [k, v] = *it;
		return &v;
	}

	bool tag_storage_iterator::operator==(const tag_storage_iterator& rhs) const
	{
		return it == rhs.it;
	}

	bool tag_storage_iterator::operator!=(const tag_storage_iterator& rhs) const
	{
		return it != rhs.it;
	}

	tag_storage_iterator::unwrapped_it tag_storage_iterator::unwrapped() const
	{
		return it;
	}
}
