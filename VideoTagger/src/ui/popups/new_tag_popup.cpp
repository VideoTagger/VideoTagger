#include "new_tag_popup.hpp"

#include <utils/random.hpp>
#include <ui/widgets/button_bar.hpp>
#include <core/app_context.hpp>

#include <events/tags/tag_add_request_event.hpp>

namespace vt::ui
{
	new_tag_popup::new_tag_popup(event_source source, std::optional<bool*> open) :
		modal_popup{ "new-tag-popup", open }, event_source_{ source }, tag_name_input_{ "##TagNameInput", "", [](const std::string& input) -> std::optional<std::string>
		{
			auto& tags = ctx_.current_project->tags;
			tag_validate_result valid_tag_name = tags.validate_tag_name(input);

			std::optional<std::string> result;
			if (valid_tag_name != tag_validate_result::ok)
			{
				switch (valid_tag_name)
				{
				case vt::tag_validate_result::already_exists: result = "Already exists"; break;
				case vt::tag_validate_result::invalid_name: result = "Invalid name"; break;
				case vt::tag_validate_result::too_long: result = fmt::format("Name can be at most {} characters long", tag_storage::max_tag_name_length); break;
				default: result = "Invalid name"; break;
				}
			}

			return result;
		}}
	{
	}

	void new_tag_popup::on_display()
	{
		set_display_name(ctx_.lang->get("popup.new_tag.title"));

		auto hue = utils::random::get_from_zero<float>();
		auto value = utils::random::get<float>(0.5f, 1.0f);
		ImVec4 color;
		ImGui::ColorConvertHSVtoRGB(hue, 0.75f, value, color.x, color.y, color.z);

		color_picker_.set_color(color);
	}

	void new_tag_popup::on_render()
	{
		auto& tags = ctx_.current_project->tags;


		tag_name_input_.render_with_label(ctx_.lang->get("popup.new_tag.tag_name"));

		if (ui::color_button("##ColorPreview", color_picker_.color(), ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip))
		{
			color_picker_.set_opened(true);
		}
		color_picker_.render("##TagColorPicker", ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);

		std::vector<std::pair<int, std::string>> buttons
		{
			{ 0, ctx_.lang->get("confirm") },
			{ 1, ctx_.lang->get("cancel") },
		};
		ui::button_bar<int>::render(buttons, tag_name_input_.is_valid(), [&](int id)
		{
				switch (id)
				{
					case 0:
					{
						ctx_.dispatch_event<tag_add_request_event>(event_source_, tags, tag_name_input_.input(), ImGui::ColorConvertFloat4ToU32(color_picker_.color()));
						close();
						break;
					}
					default: close(); break;
				}
		});
	}
}
