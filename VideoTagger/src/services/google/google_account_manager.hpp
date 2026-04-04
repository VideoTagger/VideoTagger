#pragma once
#include <services/service_account_manager.hpp>
#include <string>
#include <chrono>

namespace vt
{
	enum class obtain_token_error
	{
		request_failed,
		invalid_response,
		required_scopes_not_granted
	};

	struct obtain_token_result_data
	{
		std::string access_token;
		std::string refresh_token;
		std::vector<std::string> scope;
		std::chrono::steady_clock::time_point expire_tp;
	};

	class obtain_token_result
	{
	public:
		explicit constexpr obtain_token_result(obtain_token_error status) : data_{ status } {}
		explicit obtain_token_result(obtain_token_result_data data) : data_{ std::move(data) } {}

	private:
		std::variant<obtain_token_error, obtain_token_result_data> data_;

	public:
		constexpr obtain_token_error error() const
		{
			return std::get<obtain_token_error>(data_);
		}

		constexpr bool has_value() const
		{
			return std::holds_alternative<obtain_token_result_data>(data_);
		}

		constexpr const obtain_token_result_data& value() const
		{
			return std::get<obtain_token_result_data>(data_);
		}

		constexpr obtain_token_result_data& value()
		{
			return std::get<obtain_token_result_data>(data_);
		}
	};

	enum class get_access_token_status
	{
		success,
		refresh_failed_request_failed,
		refresh_failed_invalid_response,
		token_unavailable
	};

	struct get_access_token_result
	{
		get_access_token_status status;
		std::string access_token;
	};

	//TODO: probably shouldn't store client secret
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
		static constexpr auto static_service_id = "google";
		static constexpr auto static_service_display_name = "Google";

		google_account_manager() = default;

		std::string service_id() const override;
		std::string service_display_name() const override;

		nlohmann::ordered_json save() const override;
		void load(const nlohmann::ordered_json& json) override;

		account_properties get_account_properties_from_file(const std::filesystem::path& file_path) override;

		std::string account_name() const override;

		void on_log_out() override;

		const account_properties& get_account_properties() const override;
		void set_account_properties(const account_properties& properties) override;

		const google_account_info& account_info() const;
		account_login_status login_status() const override;

		account_login_popup_data login_popup_data() override;

		//TODO: use cancellation token
		obtain_token_result obtain_access_token(const std::string& client_id, const std::string& client_secret, bool* cancel_token);
		obtain_token_result refresh_access_token(const std::string& client_id, const std::string& client_secret, const std::string& refresh_token);
		get_access_token_result access_token();
		std::optional<std::string> obtain_user_name();
		bool revoke_token();
	
	protected:
		bool on_log_in(const account_properties& properties, bool* cancel_token) override;
		bool on_retry_login() override;
	
	private:
		static constexpr auto request_scope_ = { "https://www.googleapis.com/auth/drive.readonly", "https://www.googleapis.com/auth/userinfo.profile" };
		google_account_info account_info_;
	};
}
