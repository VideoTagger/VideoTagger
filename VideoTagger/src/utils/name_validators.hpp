#pragma once
#include <string>
#include <optional>
#include <core/localization/lang_pack.hpp>

namespace vt::utils
{
	enum class name_validation_result
	{
		ok,
		empty,
		already_exists,
		invalid
	};

	inline std::string name_validation_result_to_string(name_validation_result result, lang_pack& lang)
	{
		switch (result)
		{
		case name_validation_result::ok: return lang.get("validation.generic.valid");
		case name_validation_result::empty: return lang.get("validation.generic.empty");
		case name_validation_result::already_exists: return lang.get("validation.generic.already_exists");
		default: return lang.get("validation.generic.invalid");
		}
	}

	template<typename map_type>
	inline name_validation_result basic_map_name_validate(const std::string& name, const map_type& other_names)
	{
		if (name.empty()) return name_validation_result::empty;

		auto it = other_names.find(name);
		if (it != other_names.end()) return name_validation_result::already_exists;

		return name_validation_result::ok;
	}
}
