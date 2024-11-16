#pragma once
#include <services/service_account_manager.hpp>

namespace vt
{
	struct obtain_token_result
	{
		std::string access_token;
		std::string refresh_token;
		std::vector<std::string> scope;
		int expires_in_seconds{};
	};

	class google_account_manager : public service_account_manager
	{
	public:
		static std::string static_service_name;

		const std::string& service_name() const override;

		nlohmann::ordered_json save() const override;
		void load(const nlohmann::ordered_json& json) override;

		void options_draw_account(const std::string& account_name) override;
		bool add_popup_draw(bool& success) override;

		std::optional<obtain_token_result> obtain_token(const std::string& client_id, const std::string& client_secret);

	private:
		static constexpr auto scope = { "https://www.googleapis.com/auth/drive.readonly" };
	};

	inline std::string google_account_manager::static_service_name = "Google";
}
