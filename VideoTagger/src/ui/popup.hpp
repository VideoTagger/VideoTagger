#pragma once
#include <imgui.h>
#include <string>
#include <optional>

namespace vt::ui
{
	struct popup
	{
	public:
		popup(const std::string& id, ImGuiWindowFlags flags = ImGuiWindowFlags_None);
		virtual ~popup() = default;

	private:
		std::string id_;
		ImGuiWindowFlags flags_;

	public:
		void open(ImGuiPopupFlags flags = ImGuiPopupFlags_None);
		virtual void close();

		[[nodiscard]] const std::string& id() const;
		[[nodiscard]] ImGuiWindowFlags flags() const;

		virtual void on_render() = 0;
		void render();

	private:
		virtual bool pre_render();
		void post_render();
	};

	struct modal_popup : public popup
	{
	public:
		modal_popup(const std::string& id, std::optional<bool*> open = std::nullopt, ImGuiWindowFlags flags = ImGuiWindowFlags_None);
		virtual ~modal_popup() = default;

	private:
		std::optional<bool*> open_state_;

	public:
		virtual void close() override;
		//Makes the Escape key close the popup
		void close_on_escape();
	private:
		virtual bool pre_render() override;
	};

	template<typename popup_type, typename... arguments, typename = std::enable_if_t<std::is_base_of_v<ui::popup, popup_type> and std::is_constructible_v<popup_type, arguments...>>>
	constexpr std::unique_ptr<popup_type> new_popup(arguments&&... args)
	{
		return std::move(std::make_unique<popup_type>(std::forward<arguments>(args)...));
	}
}
