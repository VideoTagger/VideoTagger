#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include <future>
#include <nlohmann/json.hpp>

namespace vt
{
	using account_properties = nlohmann::json;

	enum class account_login_status
	{
		not_logged_in,
		logged_in,
		refresh_failed,
		expired
	};

	struct account_login_popup_field_data
	{
		std::string display_name;
		std::string property_name;
		std::string value;
	};

	struct account_login_popup_data
	{
		std::vector<account_login_popup_field_data> fields;
		bool show_file_load_button = false;

		//Don't assign
		bool cancel_token = false;
	};

	/**
	 * Define: static constexpr auto static_importer_id = "importer_id" in derived class
	 * to make app_context::register_account_manager, app_context::get_account_manager and app_context::is_account_manager_registered template funtions work.
	 */
	class service_account_manager
	{
	public:
		static std::string success_page();
		static std::string failure_page(std::string_view reason);

		service_account_manager() = default;
		virtual ~service_account_manager() = default;

		virtual std::string service_id() const = 0;
		virtual std::string service_display_name() const = 0;

		virtual std::string account_name() const = 0;

		virtual nlohmann::ordered_json save() const = 0;
		virtual void load(const nlohmann::ordered_json& json) = 0;

		virtual account_properties get_account_properties_from_file(const std::filesystem::path& file_path);

		std::future<bool> log_in(const account_properties& properties, bool* cancel_token);

		std::future<bool> retry_login();

		void log_out();
		virtual void on_log_out() = 0;

		virtual account_login_status login_status() const = 0;

		virtual const account_properties& get_account_properties() const = 0;
		virtual void set_account_properties(const account_properties& properties) = 0;

		virtual account_login_popup_data login_popup_data() = 0;

		bool draw_login_popup(bool& success);

	protected:
		//It's safe to capture the arguments by reference
		virtual bool on_log_in(const account_properties& properties, bool* cancel_token) = 0;
		virtual bool on_retry_login() = 0;
	
	private:
		std::future<bool> add_account_result_;
		std::optional<account_login_popup_data> login_popup_data_;
	};

	inline void to_json(nlohmann::ordered_json& json, const service_account_manager& manager)
	{
		json = manager.save();
	}

	inline void from_json(const nlohmann::ordered_json& json, service_account_manager& manager)
	{
		manager.load(json);
	}
}
