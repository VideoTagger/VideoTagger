#pragma once
#include <memory>
#include <core/types.hpp>
#include <unordered_map>

#include <tags/impl/attribute_instance.hpp>

namespace vt
{
	using attribute_instance_container = std::unordered_map<video_id_t, std::unordered_map<std::string, std::unique_ptr<impl::attribute_instance>>>;
}
