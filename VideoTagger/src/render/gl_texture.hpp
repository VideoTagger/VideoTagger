#pragma once
#include <SDL_opengl.h>

namespace vt
{
	class gl_texture
	{
	public:
		gl_texture(GLsizei width, GLsizei height, GLenum format, const void* pixels = nullptr, GLint filtering = GL_LINEAR);
		gl_texture(const gl_texture&) = delete;
		gl_texture(gl_texture&&) noexcept;
		~gl_texture();

	private:
		GLuint id_;
		GLsizei width_;
		GLsizei height_;
		GLenum format_;
		GLint filtering_;

	public:
		void set_pixels(void* pixels);
		void set_pixels(void* pixels, GLsizei x, GLsizei y, GLsizei width, GLsizei height, GLenum format);
		void clear();

		void bind();
		void unbind();

		GLuint id() const;
		GLsizei width() const;
		GLsizei height() const;
		GLenum format() const;

		gl_texture& operator=(const gl_texture&) = delete;
		gl_texture& operator=(gl_texture&&) noexcept;
	};
}
