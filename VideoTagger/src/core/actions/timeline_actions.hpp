#pragma once
#include <string>
#include <core/keybind_action.hpp>

namespace vt
{
	struct timestamp_action : public keybind_action
	{
		static constexpr auto action_name = "Insert Timestamp";

	public:
		timestamp_action();

	private:
		std::string tag_;

	public:
		virtual void invoke() const final override;
		virtual void to_json(nlohmann::ordered_json& json) const final override;
		virtual void from_json(const nlohmann::ordered_json& json) final override;
		virtual void render_properties() final override;
	};

	enum class segment_action_type
	{
		auto_,
		start,
		end
	};

	struct segment_action : public keybind_action
	{
		static constexpr auto action_name = "Start/End Segment";

	public:
		segment_action();

	private:
		segment_action_type type_;
		std::string tag_;

	public:
		virtual void invoke() const final override;
		virtual void to_json(nlohmann::ordered_json& json) const final override;
		virtual void from_json(const nlohmann::ordered_json& json) final override;
		virtual void render_properties() final override;
	};

	inline void to_json(nlohmann::ordered_json& json, segment_action_type action_type)
	{
		switch (action_type)
		{
			case segment_action_type::auto_: json = "auto"; break;
			case segment_action_type::start: json = "start"; break;
			case segment_action_type::end: json = "end"; break;
		}
	}

	inline void from_json(const nlohmann::ordered_json& json, segment_action_type& action_type)
	{
		if (json.is_string())
		{
			std::string json_type = json;
			if (json_type == "start")
			{
				action_type = segment_action_type::start;
				return;
			}
			else if (json_type == "end")
			{
				action_type = segment_action_type::end;
				return;
			}
		}
		action_type = segment_action_type::auto_;
	}
}
