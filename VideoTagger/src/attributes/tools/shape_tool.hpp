#pragma once
#include <imgui.h>
#include <optional>
#include <memory>
#include <type_traits>
#include <ui/toolbar/toolbar_tool.hpp>
#include <tags/tag.hpp>
#include <attributes/impl/shape.hpp>
#include <impl/resettable.hpp>
#include <core/app_context.hpp>
#include <ui/windows/toolbar.hpp>
#include <attributes/core/shape_attribute.hpp>
#include <attributes/core/shape_attribute_instance.hpp>
#include <attributes/impl/with_shape_data.hpp>

#include <events/attributes/region_insert_request_event.hpp>

namespace vt
{
	template<typename shape_type, typename = std::enable_if_t<std::is_base_of_v<impl::shape, shape_type>>>
	class shape_tool : public ui::toolbar_tool, public impl::resettable, public impl::with_shape_data<shape_type>
	{
	public:
		shape_tool(const tag& tag, const std::string& attribute_name) :
			tag_{ &tag }, attribute_name_{ attribute_name } {}

	protected:
		std::optional<video_id_t> active_video_;

	private:
		std::string attribute_name_;
		const tag* tag_;

	public:
		const std::string& attribute_name()
		{
			return attribute_name_;
		}

		const tag& get_tag() const
		{
			return *tag_;
		}

		virtual void reset() override
		{
			this->set_data(nullptr);
			active_video_.reset();
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

		bool insert_region(video_id_t video_id)
		{
			auto selected_segment_opt = ctx_.session.any_selected_segment(tag_->name);
			if (!selected_segment_opt.has_value()) return false;

			auto& segment_attr_instances = ctx_.get_current_segment_storage().at(tag_->name).segment_attribute_instances(*selected_segment_opt);
			auto& video_attr_instances = segment_attr_instances[video_id];

			auto instance_it = std::find_if(video_attr_instances.begin(), video_attr_instances.end(), [this](const auto& ptr)
			{
				return ptr != nullptr and ptr->attribute_name() == attribute_name();
			});

			if (instance_it == video_attr_instances.end())
			{
				const auto& attribute = tag_->attributes.at(attribute_name());
				video_attr_instances.push_back(attribute->instantiate());
				instance_it = video_attr_instances.end() - 1;
			}

			ctx_.dispatch_event<region_insert_request_event<shape_type>>("shape_tool", tag_->name, *selected_segment_opt, video_id, *(instance_it->get()),
				ctx_.displayed_videos.current_timestamp_as_timestamp(), *this->data());

			return true;
		}

		bool can_insert_region() const
		{
			if (!ctx_.session.is_one_segment_selected()) return false;

			auto segment_opt = ctx_.session.any_selected_segment();
			const auto& [tag_name, segment_id] = *segment_opt;
			if (tag_name != tag_->name) return false;

			auto& segments = ctx_.get_current_segment_storage().at(tag_name);
			bool result = segments
				.at(segment_id)
				.contains(ctx_.displayed_videos.current_timestamp_as_timestamp());
			return result;
		}

		///@return result of can_insert_region. Also sets mouse cursor to not-allowed if insertion is not allowed
		bool insert_allowed_cursor()
		{
			bool result = can_insert_region();
			bool is_hovered = ImGui::IsWindowHovered();
			if (is_hovered)
			{
				if (!result or is_busy())
				{
					ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
				}
			}
			return result;
		}

		virtual std::string display_name() const override
		{
			return attribute_name_;
		}

		virtual uint32_t property_column_count() const override
		{
			return toolbar_tool::property_column_count() + 1;
		}

		virtual void render_properties()
		{
			ImGui::TableNextColumn();
			if (ui::rounded_button("Done"))
			{
				on_done();
			}
		}
	};
}
