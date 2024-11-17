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

		std::string client_id() const;
		std::string client_secret() const;
		std::string access_token() const;
		std::string refresh_token() const;
		std::vector<std::string> scope() const;
		bool access_token_expired() const;
	};

	class google_account_manager : public service_account_manager
	{
	public:
		static std::string static_service_name;

		google_account_manager();

		nlohmann::ordered_json save() const override;
		void load(const nlohmann::ordered_json& json) override;

		bool on_add_account(const std::string& name, const account_properties& properties) override;
		void on_remove_account() override;

		const account_properties& get_account_properties() const override;
		void set_account_properties(const account_properties& properties) override;

		const google_account_info& account_info() const;
		bool active() const;

		void draw_options_page() override;
		bool draw_add_popup(bool& success) override;

		std::optional<obtain_token_result> obtain_token(const std::string& client_id, const std::string& client_secret);

	private:
		static constexpr auto request_scope = { "https://www.googleapis.com/auth/drive.readonly" };
		google_account_info account_info_;
	};

	inline std::string google_account_manager::static_service_name = "Google";
}
