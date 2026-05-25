#pragma once
#include <string>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <ui/toolbar/toolbar_tool_extension.hpp>

namespace vt::ui
{
	template<typename type>
	class toolbar_tool_extension_registry
	{
	public:
		using container_type = std::unordered_map<std::string, std::shared_ptr<type>>;
		using iterator = typename container_type::iterator;
		using const_iterator = typename container_type::const_iterator;

		toolbar_tool_extension_registry() = default;

	private:
		container_type extensions_;

	public:
		void register_extension(const std::string& name, std::shared_ptr<type>&& extension)
		{
			extensions_[name] = std::move(extension);
		}

		size_t size() const
		{
			return extensions_.size();
		}

		bool empty() const
		{
			return extensions_.empty();
		}

		iterator begin()
		{
			return extensions_.begin();
		}

		iterator end()
		{
			return extensions_.end();
		}

		const_iterator begin() const
		{
			return extensions_.begin();
		}

		const_iterator end() const
		{
			return extensions_.end();
		}

		std::shared_ptr<type> first()
		{
			if (extensions_.empty()) return nullptr;
			return begin()->second;
		}

		const std::shared_ptr<type> first() const
		{
			if (extensions_.empty()) return nullptr;
			return begin()->second;
		}

		std::shared_ptr<type> last()
		{
			if (extensions_.empty()) return nullptr;
			return end()->second;
		}

		const std::shared_ptr<type> last() const
		{
			if (extensions_.empty()) return nullptr;
			return end()->second;
		}

		template<typename ext_type, typename... arguments, typename = std::enable_if_t<std::is_base_of_v<type, ext_type> and std::is_constructible_v<ext_type, arguments...>>>
		std::shared_ptr<ext_type> register_extension(const std::string& name, arguments&&... args)
		{
			auto ext = std::make_shared<ext_type>(std::forward<arguments>(args)...);
			extensions_[name] = ext;
			return ext;
		}
	};
}
