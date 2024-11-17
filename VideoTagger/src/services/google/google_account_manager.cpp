#include "pch.hpp"
#include "google_account_manager.hpp"
#include <utils/oauth2.hpp>
#include <utils/string.hpp>
#include <core/debug.hpp>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>

namespace vt
{
	std::string google_account_info::client_id() const
	{
		return properties.at("client_id");
	}

	std::string google_account_info::client_secret() const
	{
		return properties.at("client_secret");
	}

	std::string google_account_info::access_token() const
	{
		return properties.at("access_token");
	}

	std::string google_account_info::refresh_token() const
	{
		return properties.at("refresh_token");
	}

	std::vector<std::string> google_account_info::scope() const
	{
		return properties.at("scope");
	}

	bool google_account_info::access_token_expired() const
	{
		return (expire_tp - std::chrono::steady_clock::now()) <= std::chrono::steady_clock::duration::zero();
	}

	google_account_manager::google_account_manager()
		: service_account_manager(static_service_name, std::nullopt)
	{
	}

	nlohmann::ordered_json google_account_manager::save() const
	{
		auto json_account = nlohmann::json::object();
		json_account["name"] = account_name();
		json_account["client_id"] = account_info_.client_id();
		json_account["client_secret"] = account_info_.client_secret();
		json_account["refresh_token"] = account_info_.refresh_token();

		return json_account;
	}

	void google_account_manager::load(const nlohmann::ordered_json& json)
	{
		account_properties properties;

		if (!json.contains("name") or !json.contains("client_id") or !json.contains("client_secret") or !json.contains("refresh_token"))
		{
			return;
		}
		properties["client_id"] = json.at("client_id");
		properties["client_secret"] = json.at("client_secret");
		properties["refresh_token"] = json.at("refresh_token");

		add_account(json.at("name"), properties);
	}

	bool google_account_manager::on_add_account(const std::string& name, const account_properties& properties)
	{
		//TODO: if refresh token is present obtain token from it

		std::string client_id = properties.at("client_id");
		std::string client_secret = properties.at("client_secret");

		auto token_result = obtain_token(client_id, client_secret);
		if (!token_result.has_value())
		{
			debug::error("Failed to obtain access token");
			return false;
		}

		account_info_.properties["client_id"] = client_id;
		account_info_.properties["client_secret"] = client_secret;

		account_info_.properties["access_token"] = token_result->access_token;
		account_info_.properties["refresh_token"] = token_result->refresh_token;
		account_info_.properties["scope"] = token_result->scope;
		account_info_.expire_tp = token_result->expire_tp;

		return true;
	}

	void google_account_manager::on_remove_account()
	{
		account_info_ = google_account_info{};
	}

	const account_properties& google_account_manager::get_account_properties() const
	{
		return account_info_.properties;
	}

	void google_account_manager::set_account_properties(const account_properties& properties)
	{
		for (auto& [prop_name, prop_value] : properties.items())
		{
			account_info_.properties[prop_name] = prop_value;
		}
	}

	const google_account_info& google_account_manager::account_info() const
	{
		return account_info_;
	}

	bool google_account_manager::active() const
	{
		return !account_info_.access_token_expired();
	}

	void google_account_manager::draw_options_page()
	{
		ImGui::Text("Active: %s", active() ? "Yes" : "No");

		std::string access_token = account_info_.access_token();
		ImGui::Text("Access token: %s", access_token.c_str());

		std::string refresh_token = account_info_.refresh_token();
		ImGui::Text("Refresh token: %s", refresh_token.c_str());
		
		std::string client_secret = account_info_.client_secret();
		ImGui::Text("Client secret: %s", client_secret.c_str());
	}

	bool google_account_manager::draw_add_popup(bool& success)
	{
		static std::string account_name;
		static std::string client_id;
		static std::string client_secret;

		ImGui::InputText("Account name", &account_name);
		ImGui::InputText("Client id", &client_id);
		ImGui::InputText("Client secret", &client_secret);
		
		if (ImGui::Button("Add"))
		{
			account_properties properties;
			properties["client_id"] = client_id;
			properties["client_secret"] = client_secret;
			success = add_account(account_name, properties);
			return true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			success = false;
			return true;
		}

		return false;
	}

	std::optional<obtain_token_result> google_account_manager::obtain_token(const std::string& client_id, const std::string& client_secret)
	{
		std::string code_verifier = utils::oauth2::generate_code_verifier();
		std::string code_challenge = utils::oauth2::generate_code_challenge(code_verifier);

		httplib::Server server;

		int redirect_port = 6060;

		std::string redirect_uri = fmt::format("http://127.0.0.1:{}/oauth2", redirect_port);

		std::optional<std::string> oauth2_code;
		{
			static constexpr auto auth_host = "https://accounts.google.com";
			std::string auth_get = fmt::format(
				"/o/oauth2/v2/auth?"
				"client_id={}&"
				"redirect_uri={}&"
				"response_type=code&"
				"scope={}&"
				"code_challenge={}&"
				"code_challenge_method=S256",
				client_id, redirect_uri, fmt::join(request_scope, " "), code_challenge
			);

			std::string auth_url = auth_host + auth_get;
			SDL_OpenURL(auth_url.c_str());

			server.Get("/oauth2", [&oauth2_code, &server](const httplib::Request& req, httplib::Response& res)
			{
				//TODO: Display a page
				res.set_content("You can now close this tab", "text/html");

				//TODO: check for errors
				oauth2_code = req.params.find("code")->second;

				server.stop();
			});

			//TODO: launch the server on a separate thread so it can be stopped
			server.listen("127.0.0.1", redirect_port);
		}

		if (!oauth2_code.has_value())
		{
			return std::nullopt;
		}

		httplib::Client client("https://oauth2.googleapis.com");
		httplib::Params params
		{
			{"client_id", client_id},
			{"client_secret", client_secret},
			{"code", *oauth2_code},
			{"code_verifier", code_verifier},
			{"grant_type", "authorization_code"},
			{"redirect_uri", redirect_uri}
		};
		auto post_res = client.Post("/token", params);
		if (!post_res)
		{
			debug::error("POST failed: {}", httplib::to_string(post_res.error()));
			return std::nullopt;
		}
		if (post_res->status != 200)
		{
			debug::error("Got response: {} {}", post_res->status, post_res->reason);
			return std::nullopt;
		}

		obtain_token_result result;

		//TODO: handle errors?
		auto json = nlohmann::json::parse(post_res->body);
		result.access_token = json.at("access_token");
		result.refresh_token = json.at("refresh_token");
		result.expire_tp = std::chrono::steady_clock::now() + std::chrono::seconds{ int(json.at("expires_in")) };
		result.scope = utils::string::split(json.at("scope"), ' ');

		return result;
	}
}
