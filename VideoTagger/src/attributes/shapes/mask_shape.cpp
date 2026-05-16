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
			mask_shape* obj{};
		};

		static mask_draw_data data;

		auto* draw_list = ImGui::GetWindowDrawList();
		//TODO: Actually get the correct video instead of just the first one
		auto& vid = *ctx_.displayed_videos.begin();
		data.texture = &vid.overlay_texture;
		data.min = draw_min;
		data.max = draw_max;
		data.fill_color = fill_color;
		data.obj = this;

		draw_list->PushClipRect(draw_min, draw_max);
		// 3. Command ImGui to switch to our target shader
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
			data->texture->set_pixels(data->obj->mask_.data(), data->obj->pos_[0], data->obj->pos_[1], data->obj->mask_.width(), data->obj->mask_.height(), GL_RED);

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

			// persistent VAO/VBO for the quad
			static GLuint quad_vao = 0;
			static GLuint quad_vbo = 0;

			if (quad_vao == 0)
			{
				glGenVertexArrays(1, &quad_vao);
				glGenBuffers(1, &quad_vbo);

				glBindVertexArray(quad_vao);
				glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
				// allocate space for 6 vertices * (pos.xy + uv.xy) = 6 * 4 floats
				glBufferData(GL_ARRAY_BUFFER, 6 * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

				/*
				// Position -> location 0 (vec2)
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0));
				// UV -> location 1 (vec2)
				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));
				*/
				int pos_loc = ctx_.shaders->mask_shader.get_attribute_location("Position");
				int uv_loc = ctx_.shaders->mask_shader.get_attribute_location("UV");
				// use pos_loc / uv_loc instead of 0/1 and handle -1 (inactive) appropriately
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
			glEnable(GL_SCISSOR_TEST);

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

			// upload verts to VBO and draw
			glBindVertexArray(quad_vao);
			glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

			auto col = ImGui::ColorConvertU32ToFloat4(data->fill_color);
			//auto col = ImGui::ColorConvertU32ToFloat4(IM_COL32_WHITE);
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

			//glBindFramebuffer(GL_FRAMEBUFFER, 0);
			//glViewport(0, 0, (GLsizei)draw_data->DisplaySize.x, (GLsizei)draw_data->DisplaySize.y);

			/*
			glBindBuffer(GL_ARRAY_BUFFER, bd->VboHandle);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bd->ElementsHandle);
			glEnableVertexAttribArray(bd->AttribLocationVtxPos);
			glEnableVertexAttribArray(bd->AttribLocationVtxUV);
			glEnableVertexAttribArray(bd->AttribLocationVtxColor);
			glVertexAttribPointer(bd->AttribLocationVtxPos, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, pos));
			glVertexAttribPointer(bd->AttribLocationVtxUV, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, uv));
			glVertexAttribPointer(bd->AttribLocationVtxColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, col));
			*/

			// ImGui passes its Projection Matrix into a global OpenGL state variable for immediate mode.
			// Depending on ImGui setup, this might require manual injection. 
			// In typical ImGui OpenGL3 backends, the uniform is named "ProjMtx".
			//GLint last_program;
			//glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
			
			//GLint tex_loc = glGetUniformLocation(last_program, "Texture");

			// NOTE: If ProjMtx fails to bind correctly, you can extract it natively from ImGui:
			// float L = ImGui::GetContext()->Viewports[0]->DrawData->DisplayPos.x; ...
		}, &data);

		// 4. Submit the Image Quad ImGui Command
		// By passing fill_color here, it arrives in mask_shader.frag as Frag_Color
		//draw_list->AddImage
		//(
		//	(ImTextureID)(intptr_t)vid.display_texture.id(),
		//	draw_min,
		//	draw_max,
		//	ImVec2{ 0.0f, 0.0f },
		//	ImVec2{ 1.0f, 1.0f },
		//	fill_color
		//);
		//ImRect bb{ draw_min, draw_max };
		//draw_list->AddImageQuad((ImTextureID)(intptr_t)vid.display_texture.id(), bb.GetBL(), bb.GetBR(), bb.GetTR(), bb.GetTL(), ImVec2(0, 0), ImVec2(1, 0), ImVec2(1, 1), ImVec2(0, 1), IM_COL32_WHITE);
		//draw_list->AddRectFilled(draw_min, draw_max, 0xFFFF00FF);
		// 5. Tell ImGui to return to its standard rendering pipeline
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
