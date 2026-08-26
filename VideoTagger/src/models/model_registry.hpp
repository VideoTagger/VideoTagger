#pragma once
#include <unordered_map>
#include <type_traits>
#include <typeinfo>

#include <core/debug.hpp>
#include <models/impl/model.hpp>

namespace vt
{
	class model_registry
	{
	public:
		model_registry() = default;
		~model_registry();

	private:
		std::unordered_map<uint64_t, std::shared_ptr<impl::model>> models_;

	public:
		template<typename type, typename... arguments, typename = std::enable_if_t<std::is_base_of_v<impl::model, type>>>
		std::shared_ptr<type> register_model(arguments&&... args)
		{
			auto& ref = models_[typeid(type).hash_code()];
			ref = std::make_shared<type>(args...);
			ref->on_register();
			debug::log("Registered model: '{}'", ref->name());
			return std::reinterpret_pointer_cast<type>(ref);
		}

		template<typename type, typename = std::enable_if_t<std::is_base_of_v<impl::model, type>>>
		std::shared_ptr<type> get_model()
		{
			auto it = models_.find(typeid(type).hash_code());
			if (it == models_.end()) return nullptr;
			return std::reinterpret_pointer_cast<type>(it->second);
		}
	};
}
