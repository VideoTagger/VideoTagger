#include "pch.hpp"
#include "ui_localization.hpp"

namespace vt
{
	template<typename loc_enum>
	localization<loc_enum>::localization() {}

	template<typename loc_enum>
	std::string& localization<loc_enum>::operator[](loc_enum id) {
		return data[id];
	}

	template<typename loc_enum>
	const std::string& localization<loc_enum>::operator[](loc_enum id) const {
		return data.at(id);
	}

	template class localization<loc_enum>;
}
