#pragma once
#include <core/theme.hpp>
#include <ui/window.hpp>

namespace vt::widgets
{
	struct theme_customizer : public ui::window
	{
	public:
		theme_customizer();

	private:
		theme original_theme;
		theme temp_theme;
		bool live_preview;

	public:
		virtual void on_display() override;
		virtual void pre_render() override;
		virtual void on_render() override;
		virtual void post_render() override;

		virtual nlohmann::ordered_json serialize() const override;
		virtual void deserialize(const nlohmann::ordered_json& json) override;
	};
}
