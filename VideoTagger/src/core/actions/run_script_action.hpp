#pragma once
#include "keybind_action.hpp"

namespace vt
{
	struct run_script_action : public keybind_action
	{
		static constexpr auto action_name = "Run Script";

	public:
		run_script_action(const std::filesystem::path& script_name = {});

	private:
		std::filesystem::path script_path_;

	public:
		virtual void invoke() const final override;
		virtual void to_json(nlohmann::ordered_json& json) const final override;
		virtual void from_json(const nlohmann::ordered_json& json) final override;
		virtual void render_properties() final override;
	};
}
