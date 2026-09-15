#include "pch.hpp"
#include "gl_shader.hpp"

#include <core/debug.hpp>

namespace vt
{
	gl_shader::gl_shader(std::string_view vertex_src, std::string_view fragment_src)
	{
		auto vertex_csrc = vertex_src.data();
		auto fragment_csrc = fragment_src.data();

		auto vertex = glCreateShader(GL_VERTEX_SHADER);
		auto fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(vertex, 1, &vertex_csrc, nullptr);
		glShaderSource(fragment, 1, &fragment_csrc, nullptr);

		glCompileShader(vertex);

		int success{};
		int log_length{};
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderiv(vertex, GL_INFO_LOG_LENGTH, &log_length);
			std::string info_log(log_length, '\0');
			glGetShaderInfoLog(vertex, static_cast<GLsizei>(info_log.size()), nullptr, info_log.data());
			debug::error("Vertex shader compilation failed: {}", info_log);
		}
		glCompileShader(fragment);
		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderiv(fragment, GL_INFO_LOG_LENGTH, &log_length);
			std::string info_log(log_length, '\0');
			glGetShaderInfoLog(fragment, static_cast<GLsizei>(info_log.size()), nullptr, info_log.data());
			debug::error("Fragment shader compilation failed: {}", info_log);
		}

		id_ = glCreateProgram();
		glAttachShader(id_, vertex);
		glDeleteShader(vertex);
		glAttachShader(id_, fragment);
		glDeleteShader(fragment);

		//glBindAttribLocation(id_, 0, "Position");
		//glBindAttribLocation(id_, 1, "UV");
		//glBindAttribLocation(id_, 2, "Color");
		//glBindFragDataLocation(id_, 0, "Out_Color");

		glLinkProgram(id_);

		glGetProgramiv(id_, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramiv(id_, GL_INFO_LOG_LENGTH, &log_length);
			std::string info_log(log_length, '\0');
			glGetProgramInfoLog(id_, static_cast<GLsizei>(info_log.size()), nullptr, info_log.data());
			debug::error("Shader program linking failed: {}", info_log);
		}
		glDetachShader(id_, vertex);
		glDetachShader(id_, fragment);
	}

	gl_shader::gl_shader(const std::string& vertex_src, const std::string& fragment_src) : gl_shader(std::string_view{ vertex_src }, std::string_view{ fragment_src })
	{
		
	}

	gl_shader::~gl_shader()
	{
		if (!is_valid()) return;

		debug::log("Deleting shader with ID {}", id_);
		glDeleteProgram(id_);
	}
	
	void gl_shader::bind() const
	{
		glUseProgram(id_);
	}

	void gl_shader::unbind() const
	{
		glUseProgram(0);
	}

	int gl_shader::get_uniform_location(const std::string& name) const
	{
		return glGetUniformLocation(id_, name.c_str());
	}

	int gl_shader::get_attribute_location(const std::string& name) const
	{
		return glGetAttribLocation(id_, name.c_str());
	}

	GLuint gl_shader::id() const
	{
		return id_;
	}

	bool gl_shader::is_valid() const
	{
		return id_ != 0;
	}
}
