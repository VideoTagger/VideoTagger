#pragma once

#include <SDL_opengl.h>

namespace vt
{
	class gl_texture
	{
	public:
		gl_texture(GLsizei width, GLsizei height, GLenum format, void* pixels = nullptr);
		gl_texture(const gl_texture&) = delete;
		gl_texture(gl_texture&&) noexcept;

		gl_texture& operator=(const gl_texture&) = delete;
		gl_texture& operator=(gl_texture&&) noexcept;
		
		~gl_texture();

		GLuint id() const;
		GLsizei width() const;
		GLsizei height() const;
		GLenum format() const;

		void set_pixels(void* pixels);

	private:
		GLuint id_;
		GLsizei width_;
		GLsizei height_;
		GLenum format_;
	};
}
