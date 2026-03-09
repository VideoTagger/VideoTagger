#include "pch.hpp"
#include "player_actions.hpp"
#include <core/app_context.hpp>
#include <widgets/video_player.hpp>

#include <events/player/seek_to_previous_frame_request_event.hpp>
#include <events/player/seek_to_next_frame_request_event.hpp>
#include <events/player/skip_next_request_event.hpp>
#include <events/player/skip_previous_request_event.hpp>
#include <events/player/playback_change_request_event.hpp>

namespace vt
{
	player_action::player_action(player_action_type type) : keybind_action(action_name), type_{ type } {}

	void player_action::invoke() const
	{
		if (!ctx_.current_project.has_value()) return;

		auto& player = ctx_.get_window<widgets::video_player>();
		const auto& callbacks = player.callbacks;
		switch (type_)
		{
			case player_action_type::play_pause:
			{
				ctx_.dispatch_event<playback_change_request_event>("player_action", player, !player.is_playing());
			}
			break;
			case player_action_type::forwards:
			{
				ctx_.dispatch_event<seek_to_next_frame_request_event>("player_action", player);
			}
			break;
			case player_action_type::backwards:
			{
				ctx_.dispatch_event<seek_to_previous_frame_request_event>("player_action", player);
			}
			break;
			case player_action_type::skip_next:
			{
				ctx_.dispatch_event<skip_next_request_event>("player_action", player);
			}
			break;
			case player_action_type::skip_previous:
			{
				ctx_.dispatch_event<skip_previous_request_event>("player_action", player);
			}
			break;
			case player_action_type::toggle_looping:
			{
				if (callbacks.on_set_looping == nullptr) break;

				switch (player.loop_mode())
				{
				case loop_mode::off:
					player.set_loop_mode(loop_mode::all);
					break;
				case loop_mode::all:
					player.set_loop_mode(loop_mode::one);
					break;
				case loop_mode::one:
					player.set_loop_mode(loop_mode::off);
					break;
				}

				callbacks.on_set_looping(player.loop_mode());
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
