#include "pch.hpp"
#include "player_actions.hpp"
#include <core/app_context.hpp>

namespace vt
{
	player_action::player_action(player_action_type type) : keybind_action(action_name), type_{ type } {}

	void player_action::invoke() const
	{
		if (!ctx_.current_project.has_value()) return;

		auto& player = ctx_.player;
		const auto& callbacks = player.callbacks;
		switch (type_)
		{
			case player_action_type::play_pause:
			{
				if (callbacks.on_set_playing == nullptr) break;
				callbacks.on_set_playing(!player.is_playing());
			}
			break;
			case player_action_type::forwards:
			{
				if (callbacks.on_seek == nullptr) break;
				callbacks.on_seek(player.data().end_ts);
			}
			break;
			case player_action_type::backwards:
			{
				if (callbacks.on_seek == nullptr) break;
				callbacks.on_seek(player.data().start_ts);
			}
			break;
			case player_action_type::skip_next:
			{
				if (callbacks.on_skip == nullptr) break;
				callbacks.on_skip(1);
			}
			break;
			case player_action_type::skip_previous:
			{
				if (callbacks.on_skip == nullptr) break;
				callbacks.on_skip(-1);
			}
			break;
			case player_action_type::toggle_looping:
			{
				if (callbacks.on_set_looping == nullptr) break;
				bool looping = !player.is_looping();
				player.set_looping(looping);
				callbacks.on_set_looping(looping);
			}
			break;
		}
	}

	void player_action::to_json(nlohmann::ordered_json& json) const
	{
		json["type"] = type_;
	}

	void player_action::from_json(const nlohmann::ordered_json& json)
	{
		if (json.contains("type"))
		{
			type_ = json.at("type");
		}
	}

	void player_action::render_properties()
	{
		ImGui::TableNextColumn();
		ImGui::Text("Type");
		ImGui::TableNextColumn();

		int* selected_type = reinterpret_cast<int*>(&type_);
		static const char* types[]{ "Play/Pause", "Seek Forwards", "Seek Backwards", "Skip To Next", "Skip To Previous", "Toggle Looping" };
		ImGui::Combo("##Type", selected_type, types, sizeof(types) / sizeof(types[0]));
	}
}
