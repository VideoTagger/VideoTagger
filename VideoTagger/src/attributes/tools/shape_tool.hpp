#pragma once
#include <imgui.h>
#include <optional>
#include <type_traits>
#include <ui/toolbar_tool.hpp>
#include <tags/tag.hpp>
#include <attributes/impl/shape.hpp>
#include <impl/resettable.hpp>
#include <core/app_context.hpp>
#include <ui/windows/toolbar.hpp>

namespace vt
{
	template<typename shape_type, typename = std::enable_if_t<std::is_base_of_v<impl::shape, shape_type>>>
	class shape_tool : public ui::toolbar_tool, public impl::resettable
	{
	public:
		shape_tool(const std::string& id, const std::string& icon, const std::string& tooltip, const tag& tag) : ui::toolbar_tool{ id, icon, tooltip }, data_{}, tag_{ &tag } {}

	private:
		std::optional<shape_type> data_;
		const tag* tag_;

	public:
		std::optional<shape_type>& data()
		{
			return data_;
		}

		const std::optional<shape_type>& data() const
		{
			return data_;
		}

		const tag& get_tag() const
		{
			return *tag_;
		}

		virtual void reset() override
		{
			data_.reset();
		}

		virtual void render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size) override
		{
			if (ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup) or !ctx_.get_window<ui::windows::toolbar>().is_visible()) return;

			if (ImGui::IsKeyPressed(ImGuiKey_Escape))
			{
				ctx_.tasks.run_on_main([this]()
				{
					reset();
					ctx_.session.toolbar.reset_active_tool("shape-tool");
				});
			}
		}
	};
}
