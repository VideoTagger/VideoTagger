#pragma once
#include <string>
#include <string_view>

#include <utils/vec.hpp>
#include <glad/glad.h>

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

		template<typename type>
		void set_uniform(const std::string& name, type value)
		{
			if constexpr (std::is_same_v<type, int>)
			{
				glUniform1i(get_uniform_location(name), value);
			}
			else if constexpr (std::is_same_v<type, float>)
			{
				glUniform1f(get_uniform_location(name), value);
			}
		}

		template<typename type>
		void set_uniform(const std::string& name, const utils::vec2<type>& value)
		{
			if constexpr (std::is_same_v<type, int>)
			{
				glUniform2i(get_uniform_location(name), value.x(), value.y());
			}
			else if constexpr (std::is_same_v<type, float>)
			{
				glUniform2f(get_uniform_location(name), value.x(), value.y());
			}
		}

		template<typename type>
		void set_uniform(const std::string& name, const utils::vec3<type>& value)
		{
			if constexpr (std::is_same_v<type, int>)
			{
				glUniform3i(get_uniform_location(name), value.x(), value.y(), value.z());
			}
			else if constexpr (std::is_same_v<type, float>)
			{
				glUniform3f(get_uniform_location(name), value.x(), value.y(), value.z());
			}
		}

		template<typename type>
		void set_uniform(const std::string& name, const utils::vec4<type>& value)
		{
			if constexpr (std::is_same_v<type, int>)
			{
				glUniform4i(get_uniform_location(name), value.x(), value.y(), value.w(), value.h());
			}
			else if constexpr (std::is_same_v<type, float>)
			{
				glUniform4f(get_uniform_location(name), value.x(), value.y(), value.w(), value.h());
			}
		}
	};
}
