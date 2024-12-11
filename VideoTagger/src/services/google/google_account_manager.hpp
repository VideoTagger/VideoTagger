#pragma once
#include <services/service_account_manager.hpp>
#include <string>
#include <chrono>

namespace vt
{
	struct obtain_token_result
	{
		std::string access_token;
		std::string refresh_token;
		std::vector<std::string> scope;
		std::chrono::steady_clock::time_point expire_tp;
	};

	struct google_account_info
	{
		account_properties properties;

		std::chrono::steady_clock::time_point expire_tp;

		std::string user_name() const;
		std::string client_id() const;
		std::string client_secret() const;
		std::string access_token() const;
		std::string refresh_token() const;
		std::vector<std::string> scope() const;

		bool access_token_expired() const;
		bool has_access_token() const;
		bool has_login_data() const;
		bool has_refresh_token() const;
	};

	class google_account_manager : public service_account_manager
	{
	public:
		static constexpr auto static_service_display_name = "Google";
		static constexpr auto static_service_id = "google";

		google_account_manager();

		nlohmann::ordered_json save() const override;
		void load(const nlohmann::ordered_json& json) override;

		void on_log_out() override;

		const account_properties& get_account_properties() const override;
		void set_account_properties(const account_properties& properties) override;

		const google_account_info& account_info() const;
		account_login_status login_status() const override;

		void draw_options_page() override;
		bool draw_login_popup(bool& success) override;

		//TODO: error messages in the return value would be nice, would require something like std::expected
		std::optional<obtain_token_result> obtain_access_token(const std::string& client_id, const std::string& client_secret, bool* cancel_token);
		std::optional<obtain_token_result> refresh_access_token(const std::string& client_id, const std::string& client_secret, const std::string& refresh_token);
		std::optional<std::string> access_token();
		std::optional<std::string> obtain_user_name();
		bool revoke_token();
	
	protected:
		bool on_log_in(const account_properties& properties, bool* cancel_token) override;
	
	private:
		static constexpr auto request_scope_ = { "https://www.googleapis.com/auth/drive.readonly", "https://www.googleapis.com/auth/userinfo.profile" };
		google_account_info account_info_;
		std::future<bool> add_account_result_;
	};
}
