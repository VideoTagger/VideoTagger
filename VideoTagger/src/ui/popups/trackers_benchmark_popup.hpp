#pragma once
#include <vector>
#include <optional>
#include <string>
#include <utility>

#include <ui/popup.hpp>
#include <ui/widgets/text_input.hpp>

namespace vt::ui
{
	struct trackers_benchmark_popup : public modal_popup
	{
	public:
		trackers_benchmark_popup(std::optional<bool*> open = std::nullopt);

	private:
		std::vector<std::pair<std::string, bool>> trackers_;
		ui::text_input dataset_path_input_;
		ui::text_input result_path_input_;

	public:
		virtual void on_display() override;
		virtual void on_render() override;
	};
}
