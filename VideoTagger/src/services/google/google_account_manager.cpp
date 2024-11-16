#include "pch.hpp"
#include "google_account_manager.hpp"
#include <utils/oauth2.hpp>
#include <utils/string.hpp>
#include <core/debug.hpp>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>

namespace vt
{
	const std::string& google_account_manager::service_name() const
	{
		return static_service_name;
	}

	nlohmann::ordered_json google_account_manager::save() const
	{
		auto json_accounts = nlohmann::json::array();
		for (auto& [name, info] : accounts())
		{
			auto json_account_info = nlohmann::json::object();
			json_account_info["name"] = name;
			for (auto& [prop_name, prop_value] : info.properties.items())
			{
				json_account_info[prop_name] = prop_value;
			}

			json_accounts.push_back(json_account_info);
		}

		return json_accounts;
	}

	void google_account_manager::load(const nlohmann::ordered_json& json)
	{
		for (auto& json_account_info : json)
		{
			if (!json_account_info.contains("name"))
			{
				continue;
			}

			std::string name = json_account_info.at("name");
			account_info info;
			//TODO: refresh token
			info.active = true;
			for (auto& [json_prop_name, json_prop_value] : json_account_info.items())
			{
				if (json_prop_name == "name")
				{
					continue;
				}

				info.properties[json_prop_name] = json_prop_value;
			}

			add_account(name, info);
		}
	}

	void google_account_manager::options_draw_account(const std::string& account_name)
	{
		auto& info = get_account_info(account_name);

		ImGui::Text("Active: %s", info.active ? "Yes" : "No");

		std::string access_token = info.properties.contains("access_token") ? info.properties.at("access_token") : "No Token";
		ImGui::Text("Access token: %s", access_token.c_str());

		std::string refresh_token = info.properties.contains("refresh_token") ? info.properties.at("refresh_token") : "No Token";
		ImGui::Text("Refresh token: %s", refresh_token.c_str());
		
		std::string secret = info.properties["client_secret"];
		ImGui::Text("Client secret: %s", secret.c_str());
	}

	bool google_account_manager::add_popup_draw(bool& success)
	{
		static std::string account_name;
		static std::string client_id;
		static std::string client_secret;

		ImGui::InputText("Account name", &account_name);
		ImGui::InputText("Client id", &client_id);
		ImGui::InputText("Client secret", &client_secret);
		
		if (ImGui::Button("Add"))
		{
			auto token_result = obtain_token(client_id, client_secret);
			if (!token_result.has_value())
			{
				debug::error("Failed to obtain access token");
				success = false;
				return true;
			}

			account_info info;
			info.active = true;
			info.properties["access_token"] = token_result->access_token;
			info.properties["refresh_token"] = token_result->refresh_token;
			info.properties["token_expires_in"] = token_result->expires_in_seconds;
			info.properties["scope"] = token_result->scope;
			info.properties["client_id"] = client_id;
			info.properties["client_secret"] = client_secret;
			success = add_account(account_name, info);
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
				client_id, redirect_uri, fmt::join(scope, " "), code_challenge
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
		result.expires_in_seconds = json.at("expires_in");
		result.scope = utils::string::split(json.at("scope"), ' ');

		return result;
	}
}
