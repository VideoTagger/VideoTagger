#include "mask_shape.hpp"
#include <core/debug.hpp>
#include <core/app_context.hpp>

#include <backends/imgui_impl_opengl3.h>

namespace vt
{
	mask_shape::mask_shape(int width, int height) : mask_{ width, height }
	{

	}

	mask_shape::mask_shape(const image<image_pixel_format::gray8>& mask) : mask_{ mask }
	{

	}

	bool mask_shape::operator==(const mask_shape& other) const
	{
		return this == &other;
	}

	utils::vec4<int> mask_shape::bounding_box() const
	{
		return { pos_[0], pos_[1], pos_[0] + mask_.width(), pos_[1] + mask_.height() };
	}

	void mask_shape::set_target(event_source source, video_id_t video_id)
	{

	}

	bool mask_shape::contains(utils::vec2<int> point) const
	{
		auto bb = bounding_box();
		if (point[0] < bb[0] or  point[0] > bb[2] or point[1] < bb[1] or point[1] > bb[3])
		{
			return false;
		}
		return mask_.at(point).value > 0;
	}

	utils::vec2<int>* mask_shape::closest_point(utils::vec2<int> point, float max_distance)
	{
		return nullptr;
	}

	std::vector<utils::vec2<int>*> mask_shape::get_all_points()
	{
		return {};
	}

	void mask_shape::render_shape(utils::vec2<int> shape_space, ImVec2 draw_min, ImVec2 draw_max, uint32_t fill_color, uint32_t outline_color)
	{
		struct mask_draw_data
		{
			gl_texture* texture{};
			ImVec2 min;
			ImVec2 max;
			uint32_t fill_color{};
			mask_shape obj{};
		};

		mask_draw_data* data = new mask_draw_data;

		auto* draw_list = ImGui::GetWindowDrawList();
		//TODO: Actually get the correct video instead of just the first one
		auto& vid = *ctx_.displayed_videos.begin();
		data->texture = &vid.overlay_texture;
		data->min = draw_min;
		data->max = draw_max;
		data->fill_color = fill_color;
		data->obj = *this;

		draw_list->PushClipRect(draw_min, draw_max);
		draw_list->AddCallback([](const ImDrawList* parent_list, const ImDrawCmd* cmd)
		{
			auto* data = static_cast<mask_draw_data*>(cmd->UserCallbackData);
			auto& mask_shader = ctx_.shaders->mask_shader;
			if (!mask_shader.is_valid()) return;

			auto* draw_data = ImGui::GetDrawData();

			float L = draw_data->DisplayPos.x;
			float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
			float T = draw_data->DisplayPos.y;
			float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;

			data->texture->clear();
			data->texture->set_pixels(data->obj.mask_.data(), data->obj.pos_[0], data->obj.pos_[1], data->obj.mask_.width(), data->obj.mask_.height(), GL_RED);

			const float ortho_projection[4][4] =
			{
				{ 2.0f / (R - L),   0.0f,         0.0f,   0.0f },
				{ 0.0f,         2.0f / (T - B),   0.0f,   0.0f },
				{ 0.0f,         0.0f,        -1.0f,   0.0f },
				{ (R + L) / (L - R),  (T + B) / (B - T),  0.0f,   1.0f },
			};

			mask_shader.bind();
			GLint current_prog = 0;
			glGetIntegerv(GL_CURRENT_PROGRAM, &current_prog);
			if (current_prog == 0 or current_prog != mask_shader.id())
			{
				debug::error("Shader wasn't bound correctly! Expected {}, got {}", mask_shader.id(), current_prog);
			}

			static GLuint quad_vao = 0;
			static GLuint quad_vbo = 0;

			if (quad_vao == 0)
			{
				glGenVertexArrays(1, &quad_vao);
				glGenBuffers(1, &quad_vbo);

				glBindVertexArray(quad_vao);
				glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
				glBufferData(GL_ARRAY_BUFFER, 6 * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
								int pos_loc = ctx_.shaders->mask_shader.get_attribute_location("Position");
				int uv_loc = ctx_.shaders->mask_shader.get_attribute_location("UV");
				glEnableVertexAttribArray(pos_loc);
				glVertexAttribPointer(pos_loc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(uv_loc);
				glVertexAttribPointer(uv_loc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
			}

			auto proj_loc = mask_shader.get_uniform_location("ProjMtx");
			if (proj_loc >= 0) glUniformMatrix4fv(proj_loc, 1, GL_FALSE, &ortho_projection[0][0]);
			auto tex_loc = mask_shader.get_uniform_location("Texture");
			if (tex_loc >= 0)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, data->texture->id());
				glUniform1i(tex_loc, 0);
			}

			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			glDisable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_STENCIL_TEST);
			glDisable(GL_SCISSOR_TEST);

			ImRect quad{ data->min, data->max };
			// 6 vertices, each: pos.x, pos.y, u, v
			float verts[6 * 4]
			{
				quad.GetBL().x, quad.GetBL().y, 0.0f, 1.0f, // BL
				quad.GetBR().x, quad.GetBR().y, 1.0f, 1.0f, // BR
				quad.GetTR().x, quad.GetTR().y, 1.0f, 0.0f, // TR

				quad.GetBL().x, quad.GetBL().y, 0.0f, 1.0f, // BL
				quad.GetTR().x, quad.GetTR().y, 1.0f, 0.0f, // TR
				quad.GetTL().x, quad.GetTL().y, 0.0f, 0.0f  // TL
			};

			glBindVertexArray(quad_vao);
			glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

			auto col = ImGui::ColorConvertU32ToFloat4(data->fill_color);
			auto col_loc = mask_shader.get_attribute_location("Color");
			if (col_loc >= 0)
			{
				glVertexAttrib4f(col_loc, col.x, col.y, col.z, col.w);
			}

			glDrawArrays(GL_TRIANGLES, 0, 6);

			// cleanup
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
			mask_shader.unbind();
			delete data;

			//glBindFramebuffer(GL_FRAMEBUFFER, 0);
			//glViewport(0, 0, (GLsizei)draw_data->DisplaySize.x, (GLsizei)draw_data->DisplaySize.y);

		}, data);

		draw_list->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
		draw_list->PopClipRect();
	}

	[[nodiscard]] nlohmann::ordered_json mask_shape::serialize() const
	{
		nlohmann::ordered_json json;
		json["position"] = pos_;
		return json;
	}

	void mask_shape::deserialize(const nlohmann::ordered_json& json)
	{
		if (!json.contains("position"))
		{
			debug::error("Invalid JSON: missing 'position' field");
			return;
		}
		pos_ = json["position"].get<utils::vec2<int>>();
	}
}
