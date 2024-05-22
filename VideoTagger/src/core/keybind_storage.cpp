#include "pch.hpp"
#include "keybind_storage.hpp"

namespace vt
{
	std::pair<keybind_storage::iterator, bool> keybind_storage::insert(const std::string& name, const keybind& keybind)
	{
		return keybinds_.insert({ name, keybind });
	}

	size_t keybind_storage::erase(const std::string& name)
	{
		return keybinds_.erase(name);
	}

	std::pair<keybind_storage::iterator, bool> keybind_storage::rename(const std::string& current_name, const std::string& new_name)
	{
		auto current_it = keybinds_.find(current_name);
		if (current_it == keybinds_.end())
		{
			return { keybinds_.end(), false };
		}

		if (auto it = keybinds_.find(new_name); it != keybinds_.end())
		{
			return { it, false };
		}

		auto node_handle = keybinds_.extract(current_name);
		node_handle.key() = new_name;
		auto insert_return = keybinds_.insert(std::move(node_handle));

		return { iterator{ insert_return.position }, true };
	}

	keybind& keybind_storage::at(const std::string& name)
	{
		return keybinds_.at(name);
	}

	const keybind& keybind_storage::at(const std::string& name) const
	{
		return keybinds_.at(name);
	}

	void keybind_storage::clear()
	{
		keybinds_.clear();
	}

	bool keybind_storage::contains(const std::string& name) const
	{
		return keybinds_.find(name) != keybinds_.end();
	}

	bool keybind_storage::is_valid(const std::string& name, const keybind& keybind) const
	{
		auto it = std::find_if(keybinds_.begin(), keybinds_.end(), [&](const std::pair<std::string, vt::keybind>& kb)
		{
			return kb.first == name or kb.second == keybind;
		});
		return it == keybinds_.end() and keybind.key_code >= 0;
	}

	size_t keybind_storage::size() const
	{
		return keybinds_.size();
	}

	bool keybind_storage::empty() const
	{
		return keybinds_.empty();
	}

	keybind& keybind_storage::operator[](const std::string& name)
	{
		return at(name);
	}

	const keybind& keybind_storage::operator[](const std::string& name) const
	{
		return at(name);
	}

	keybind_storage::iterator keybind_storage::begin()
	{
		return keybinds_.begin();
	}

	keybind_storage::const_iterator keybind_storage::begin() const
	{
		return keybinds_.begin();
	}

	keybind_storage::const_iterator keybind_storage::cbegin() const
	{
		return keybinds_.cbegin();
	}

	keybind_storage::iterator keybind_storage::end()
	{
		return keybinds_.end();
	}

	keybind_storage::const_iterator keybind_storage::end() const
	{
		return keybinds_.end();
	}

	keybind_storage::const_iterator keybind_storage::cend() const
	{
		return keybinds_.cend();
	}
}
