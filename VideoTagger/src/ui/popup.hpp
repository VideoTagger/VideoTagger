#pragma once
#include <imgui.h>
#include <string>
#include <optional>

namespace vt::ui
{
	///@brief Base class for a popup
	struct popup
	{
	public:
		/**
		 * @brief Creates a popup with the given id and flags
		 * @param[in] id The id for the popup
		 * @param[in] flags The window flags used when rendering the popup
		 */
		popup(const std::string& id, ImGuiWindowFlags flags = ImGuiWindowFlags_None);
		virtual ~popup() = default;

	private:
		std::string id_;
		ImGuiWindowFlags flags_;

	public:
		/**
		 * @brief Opens the popup
		 * @param[in] flags Popup flags passed into ImGui::OpenPopup()
		 */
		void open(ImGuiPopupFlags flags = ImGuiPopupFlags_None);
		///@brief Closes the popup
		virtual void close();

		///@return The id of the popup
		[[nodiscard]] const std::string& id() const;
		///@return The flags used when rendering the popup
		[[nodiscard]] ImGuiWindowFlags flags() const;

		///@brief Called when the popup is appearing
		virtual void on_display();
		///@brief Called when the popup is being rendered
		virtual void on_render() = 0;
		/**
		 * @brief Opens the popup if the condition is true and renders it
		 * @param[in] condition The condition to check
		 * @param[in] flags Popup flags passed into popup::open()
		 */
		void open_and_render(bool condition, ImGuiPopupFlags flags = 0);
		void render();

	protected:
		/**
		 * @brief Begins the popup rendering context
		 * @return true if the popup is open and should be rendered, false otherwise
		 */
		virtual bool pre_render();
		/// @brief Ends the popup rendering context
		virtual void post_render();
		/// @brief Renders content after the window title is rendered
		virtual void post_title_render();
	};

	///@brief Base class for a modal popup
	struct modal_popup : public popup
	{
	public:
		/**
		 * @brief Creates a modal popup with the given id, open state and flags
		 * @param[in] id The id for the popup
		 * @param[in,out] open If boolean pointer is passed, it will be set to true when the popup is open or false otherwise.
		 * If std::nullopt is passed, the popup will still be able to be closed but you won't know the state of the popup.
		 * If nullptr is passed, the popup won't be able to be closed.
		 * @param[in] flags
		 */
		modal_popup(const std::string& id, std::optional<bool*> open = std::nullopt, ImGuiWindowFlags flags = ImGuiWindowFlags_None);
		virtual ~modal_popup() = default;

	private:
		std::optional<bool*> open_state_;

	public:
		///@brief Closes the popup
		virtual void close() override;
		///@brief Makes the Escape key close the popup when pressed
		void close_on_escape();
	protected:
		///@brief Applies the default modal popup style and begins the popup rendering context
		virtual bool pre_render() override;

		virtual void post_title_render() override;
	};

	/**
	 * @brief Creates a new popup of the given type
	 * @param[in] args Constructor arguments for the popup
	 * @return Created popup
	 */
	template<typename popup_type, typename... arguments, typename = std::enable_if_t<std::is_base_of_v<ui::popup, popup_type> and std::is_constructible_v<popup_type, arguments...>>>
	constexpr std::unique_ptr<popup_type> new_popup(arguments&&... args)
	{
		return std::move(std::make_unique<popup_type>(std::forward<arguments>(args)...));
	}
}
