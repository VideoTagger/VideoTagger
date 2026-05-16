#pragma once
#include <string>
#include <string_view>

#include <SDL_opengl.h>

namespace vt
{
	class gl_shader
	{
	public:
		gl_shader() = default;
		gl_shader(std::string_view vertex_src, std::string_view fragment_src);
		gl_shader(const std::string& vertex_src, const std::string& fragment_src);
		~gl_shader();

	private:
		GLuint id_ = 0;

	public:
		void bind() const;
		void unbind() const;

		int get_uniform_location(const std::string& name) const;
		int get_attribute_location(const std::string& name) const;

		GLuint id() const;
		bool is_valid() const;
	};
}
