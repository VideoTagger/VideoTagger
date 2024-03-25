#pragma once
#include <map>
#include <string>

#include "input.hpp"

namespace vt
{
	class keybind_storage
	{
	public:
		using container = std::map<std::string, keybind>;
		using iterator = container::iterator;
		using const_iterator = container::const_iterator;

		keybind_storage() = default;

	private:
		container keybinds_;

	public:
		std::pair<iterator, bool> insert(const std::string& name, const keybind& keybind);
		size_t erase(const std::string& name);
		std::pair<iterator, bool>  rename(const std::string& current_name, const std::string& new_name);
		keybind& at(const std::string& name);
		const keybind& at(const std::string& name) const;
		
		void clear();

		size_t size() const;
		bool empty() const;

		keybind& operator[](const std::string& name);
		const keybind& operator[](const std::string& name) const;

		iterator begin();
		const_iterator begin() const;
		const_iterator cbegin() const;
		iterator end();
		const_iterator end() const;
		const_iterator cend() const;
	};
}
