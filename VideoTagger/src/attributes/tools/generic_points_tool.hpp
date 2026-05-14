#pragma once
#include "shape_tool.hpp"
#include <algorithm>
#include <utils/vec.hpp>
#include <core/app_context.hpp>
#include <utils/math.hpp>
#include <utils/intersection.hpp>

namespace vt
{
	template<typename shape_type>
	class generic_points_tool : public shape_tool<shape_type>
	{
	public:
		generic_points_tool(const tag& tag, const std::string& attribute_name) :
			shape_tool<shape_type>{ tag, attribute_name } {}

		virtual void render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size) override
		{
			shape_tool<shape_type>::render_overlay(video_id, pos, size, tex_size);

			auto& shape_data = this->data();

			static auto to_texture_space = [](const ImVec2& screen_pos, ImVec2 pos, ImVec2 size, ImVec2 tex_size) -> utils::vec2<int>
			{
				return math::scale_vec2(screen_pos, pos, pos + size, utils::vec2<int>{}, utils::vec2<int>{ static_cast<int>(tex_size.x), static_cast<int>(tex_size.y) }, false);
			};

			bool is_active_video = ImGui::IsWindowHovered() and (!this->active_video_.has_value() or *this->active_video_ == video_id);
			bool insert_allowed = this->insert_allowed_cursor() and is_active_video;

			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) and insert_allowed)
			{
				auto mouse_pos = ImGui::GetMousePos();
				//if (utils::intersection::is_in_rect(mouse_pos, ImRect{ pos, pos + size }))
				//{
					if (!shape_data.has_value())
					{
						shape_data.emplace();
					}

					shape_data->points.push_back(to_texture_space(mouse_pos, pos, size, tex_size));
					this->active_video_ = video_id;
				//}
			}

			if (shape_data.has_value() and is_active_video)
			{
				const auto& tag = this->get_tag();
				shape_data->render(utils::vec2<int>({ static_cast<int>(tex_size.x), static_cast<int>(tex_size.y) }), pos, pos + size, tag.fill_color(), tag.outline_color(), 3.f);

				if (ImGui::IsKeyPressed(ImGuiKey_Enter) and insert_allowed)
				{
					on_done();
				}

			}
			if (ImGui::IsKeyPressed(ImGuiKey_Escape))
			{
				this->reset();
			}
		}

		virtual void on_done() override
		{
			if (!this->active_video_.has_value())
			{
				this->reset();
				return;
			}

			auto& shape_data = this->data();

			if (!shape_data->points.empty())
			{
				this->insert_region(*this->active_video_);
			}

			this->reset();
		}
	};
}
