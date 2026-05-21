#pragma once
#include <memory>
#include <variant>
#include <impl/resettable.hpp>
#include <attributes/shapes/mask_shape.hpp>
#include <attributes/shapes/rectangle_shape.hpp>
#include <attributes/tools/extensions/impl/wand_tool_extension.hpp>

namespace vt::ui
{
	struct wand_graph_cut_extension : public impl::wand_tool_extension, public vt::impl::resettable
	{
	public:
		wand_graph_cut_extension();

	private:
		std::variant<std::unique_ptr<rectangle_shape>, std::unique_ptr<mask_shape>> mode_data_;

	public:
		bool is_rect_mode() const;
		bool is_mask_mode() const;

		void generate_mask(video_id_t video_id);

		virtual void reset() override;

		virtual void render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size) override;
		virtual void render_properties() override;
	};
}
