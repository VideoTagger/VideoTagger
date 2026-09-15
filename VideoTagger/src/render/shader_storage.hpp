#pragma once
#include <render/gl_shader.hpp>

namespace vt
{
	class shader_storage
	{
	public:
		shader_storage();
		shader_storage(const shader_storage&) = delete;
		shader_storage(shader_storage&&) = default;

	public:
		gl_shader mask_shader;
		gl_shader mask_preview_shader;

	public:
		shader_storage& operator=(shader_storage&&) = default;
		shader_storage& operator=(const shader_storage&) = delete;
	};
}
