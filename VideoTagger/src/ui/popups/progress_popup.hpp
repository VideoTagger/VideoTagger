#pragma once
#include <ui/popup.hpp>

namespace vt::ui
{
	struct progress_popup : public modal_popup
	{
	public:
		progress_popup(const std::string& description, std::function<std::optional<float>(progress_popup&)> progress_callback, std::function<bool(const std::optional<float>& progress)> should_close, std::function<void()> on_cancel, std::optional<bool*> open = std::nullopt);

	private:
		std::function<bool(const std::optional<float>& progress)> should_close_;
		std::function<void()> on_cancel_;
		std::function<std::optional<float>(progress_popup&)> progress_callback_;
		std::string description_;
		std::optional<float> progress_value_;
		float elapsed_acc_{};
		uint8_t dot_count_{};

	public:
		void set_description(const std::string& description);

		virtual void pre_style() override;

		virtual void on_render() override;

		virtual void on_close() override;
	};
}
