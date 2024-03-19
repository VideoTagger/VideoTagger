#pragma once
#include <string>
#include <core/keybind_action.hpp>

namespace vt
{
	struct add_timestamp_action : public keybind_action
	{
	public:
		add_timestamp_action();

	private:
		std::string tag_;

	public:
		virtual void invoke() const final override;
		virtual void render_properties(bool compact) final override;
	};

	enum class segment_action_type
	{
		auto_,
		start,
		end
	};

	struct segment_action : public keybind_action
	{
	public:
		segment_action();

	private:
		segment_action_type type_;
		std::string tag_;

	public:
		virtual void invoke() const final override;
		virtual void render_properties(bool compact) final override;
	};
}
