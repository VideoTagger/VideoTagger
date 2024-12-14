#pragma once
#include "keybind_action.hpp"

namespace vt
{
	enum class player_action_type
	{
		play_pause,
		forwards,
		backwards,
		skip_next,
		skip_previous,
		toggle_looping
	};

	struct player_action : public keybind_action
	{
		static constexpr auto action_name = "Player Action";

	public:
		player_action(player_action_type type = player_action_type::play_pause);

	private:
		player_action_type type_;

	public:
		virtual void invoke() const final override;
		virtual void to_json(nlohmann::ordered_json& json) const final override;
		virtual void from_json(const nlohmann::ordered_json& json) final override;
		virtual void render_properties() final override;
	};

	inline void to_json(nlohmann::ordered_json& json, player_action_type action_type)
	{
		switch (action_type)
		{
			case player_action_type::play_pause: json = "play-pause"; break;
			case player_action_type::forwards: json = "seek-forwards"; break;
			case player_action_type::backwards: json = "seek-backwards"; break;
			case player_action_type::skip_next: json = "skip-next"; break;
			case player_action_type::skip_previous: json = "skip-previous"; break;
			case player_action_type::toggle_looping: json = "toggle-looping"; break;
		}
	}

	inline void from_json(const nlohmann::ordered_json& json, player_action_type& action_type)
	{
		if (json.is_string())
		{
			std::string json_type = json;
			if (json_type == "seek-forwards")
			{
				action_type = player_action_type::backwards;
				return;
			}
			else if (json_type == "seek-backwards")
			{
				action_type = player_action_type::backwards;
				return;
			}
			else if (json_type == "skip-next")
			{
				action_type = player_action_type::skip_next;
				return;
			}
			else if (json_type == "skip-previous")
			{
				action_type = player_action_type::skip_previous;
				return;
			}
			else if (json_type == "toggle-looping")
			{
				action_type = player_action_type::toggle_looping;
				return;
			}
		}
		action_type = player_action_type::play_pause;
	}
}

