#include "pch.hpp"
#include "gl_texture.hpp"

namespace vt
{
	gl_texture::gl_texture(GLsizei width, GLsizei height, GLenum format, void* pixels) : id_{}, width_{ width }, height_{ height }, format_{ format }
	{
		glGenTextures(1, &id_);
		glBindTexture(GL_TEXTURE_2D, id_);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	gl_texture::gl_texture(gl_texture&& other) noexcept : width_{ other.width_ }, height_{ other.height_ }, id_{ other.id_ }
	{
		other.id_ = 0;
	}

	gl_texture& gl_texture::operator=(gl_texture&& other) noexcept
	{
		if (id_ != 0)
		{
			glDeleteTextures(1, &id_);
		}

		width_ = other.width_;
		height_ = other.height_;
		id_ = other.id_;

		other.id_ = 0;

		return *this;
	}

	gl_texture::~gl_texture()
	{
		if (id_ != 0)
		{
			glDeleteTextures(1, &id_);
		}
	}

	GLuint gl_texture::id() const
	{
		return id_;
	}

	GLsizei gl_texture::width() const
	{
		return width_;
	}

	GLsizei gl_texture::height() const
	{
		return height_;
	}

	GLenum gl_texture::format() const
	{
		return format_;
	}

	void gl_texture::set_pixels(void* pixels)
	{
		glBindTexture(GL_TEXTURE_2D, id_);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_, format_, GL_UNSIGNED_BYTE, pixels);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}
