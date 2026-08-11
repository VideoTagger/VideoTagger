#include "wand_tool.hpp"
#include <core/app_context.hpp>

namespace vt
{
	wand_tool::wand_tool(const tag& tag, const std::string& attribute_name) : shape_tool<mask_shape>{ tag, attribute_name }, extension_combo_{ "##ExtensionCombo", {} } {}

	void wand_tool::on_activate()
	{
		on_switch_context();
	}

	void wand_tool::on_switch_context()
	{
		auto ext = ctx_.wand_extensions.first();
		if (ext == nullptr) return;
		ext->set_data(this->data());
		switch_extension(ext);

		auto extensions = extension_names();
		extension_combo_.set_items(extensions);

		extension_combo_.set_callback([this](const std::pair<size_t, const std::string&>& item)
		{
			auto it = std::find_if(ctx_.wand_extensions.begin(), ctx_.wand_extensions.end(), [&item](const auto& pair)
			{
				return pair.second->name() == item.second;
			});
			if (it != ctx_.wand_extensions.end())
			{
				switch_extension(it->second);
			}
		});
	}

	void wand_tool::render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size)
	{
		set_has_body(!ctx_.wand_extensions.empty());
		shape_tool<mask_shape>::render_overlay(video_id, pos, size, tex_size);

		auto shape_data = data();
		const auto& tag = get_tag();
		bool is_video_active = active_video_.has_value() and *active_video_ == video_id;

		if (shape_data != nullptr and is_video_active)
		{
			auto zoom_factor = 1.f / std::min(tex_size.x / size.x, tex_size.y / size.y);
			shape_data->render_shape_ex(utils::vec2<int>({ static_cast<int>(tex_size.x), static_cast<int>(tex_size.y) }), ImRect{ pos, pos + size }, tag.fill_color(true), tag.outline_color(true), video_id, true, zoom_factor);
		}

		bool can_insert = insert_allowed_cursor();

		auto ext = active_extension();
		if (ext == nullptr) return;

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) and ImGui::IsWindowHovered() and can_insert)
		{
			auto vid_it = ctx_.displayed_videos.find(video_id);
			if (vid_it == ctx_.displayed_videos.end()) return;

			auto& stream = vid_it->video;
			if (shape_data == nullptr)
			{
				auto ptr = std::make_shared<mask_shape>(stream.width(), stream.height());
				ext->set_data(ptr);
				set_data(ptr);
			}
			else if (!is_video_active)
			{
				shape_data->mask.clear();
			}
			active_video_ = video_id;
			is_video_active = true;
		}

		if (is_video_active)
		{
			if (can_insert)
			{
				ext->render_overlay(video_id, pos, size, tex_size);
			}
			set_busy(ext->is_busy());
		}

		if (ImGui::IsKeyPressed(ImGuiKey_Enter))
		{
			on_done();
		}
	}

	void wand_tool::on_done()
	{
		if (!can_insert_region()) return;

		auto shape_data = data();
		if (!active_video_.has_value() or shape_data == nullptr)
		{
			reset();
			return;
		}

		if (!shape_data->mask.empty())
		{
			shape_data->recalculate_bounding_box();
			insert_region(*active_video_);
		}

		auto ext = active_extension();
		if (ext != nullptr)
		{
			ext->on_done();
		}
		reset();
	}

	uint32_t wand_tool::property_column_count() const
	{
		auto col_count = shape_tool<mask_shape>::property_column_count();
		auto ext = active_extension();
		if (ext != nullptr)
		{
			col_count += ext->property_column_count();
		}
		return col_count;
	}

	void wand_tool::render_properties()
	{
		auto ext = active_extension();
		if (ext != nullptr)
		{
			ext->render_properties();
		}
		shape_tool<mask_shape>::render_properties();
	}

	void wand_tool::render_popup_body(ui::widget_list& widgets, ui::button_bar<int>& button_bar)
	{
		widgets.add_raw([this]()
		{
			auto extensions = extension_names();
			if (active_extension() != nullptr)
			{
				auto it = std::find(extensions.begin(), extensions.end(), active_extension()->name());
				if (it != extensions.end())
				{
					extension_combo_.set_selected(it - extensions.begin());
				}
			}
			return extension_combo_.render_with_label("Method");
		});
	}

	void wand_tool::on_switch_extension(std::shared_ptr<ui::impl::wand_tool_extension> new_extension)
	{
		if (new_extension == nullptr) return;

		new_extension->set_data(data());
	}

	std::vector<std::string> wand_tool::extension_names() const
	{
		std::vector<std::string> result;
		for (const auto& [id, ext] : ctx_.wand_extensions)
		{
			result.push_back(ext->name());
		}
		return result;
	}
}

