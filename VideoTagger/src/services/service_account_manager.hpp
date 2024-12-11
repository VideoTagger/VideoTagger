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

	class service_account_manager
	{
	public:
		static std::string success_page();
		static std::string failure_page(std::string_view reason);

		service_account_manager(std::string service_id, std::string service_display_name);
		virtual ~service_account_manager() = default;

		const std::string& service_id() const;
		const std::string& service_display_name() const;

		virtual std::string account_name() const = 0;

		virtual nlohmann::ordered_json save() const = 0;
		virtual void load(const nlohmann::ordered_json& json) = 0;

		//TODO: maybe use std::any instead of nlohmann::json
		std::future<bool> log_in(const account_properties& properties, bool* cancel_token);

		std::future<bool> retry_login();

		void log_out();
		virtual void on_log_out() = 0;

		virtual account_login_status login_status() const = 0;

		virtual const account_properties& get_account_properties() const = 0;
		virtual void set_account_properties(const account_properties& properties) = 0;

		//TODO: maybe do the drawing in the parent class and let the children determine field names
		//return true if the popup is ready to be closed
		virtual bool draw_login_popup(bool& success) = 0;

	protected:
		//It's safe to capture the arguments by reference
		virtual bool on_log_in(const account_properties& properties, bool* cancel_token) = 0;
		virtual bool on_retry_login() = 0;
	
	private:
		std::string service_id_;
		std::string service_display_name_;
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
