#include "pch.hpp"
#include "google_account_manager.hpp"
#include <utils/oauth2.hpp>
#include <utils/string.hpp>
#include <core/debug.hpp>
#include <utils/random.hpp>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>

namespace vt
{
	std::string google_account_info::user_name() const
	{
		return properties.at("user_name");
	}

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

	bool google_account_info::has_access_token() const
	{
		return properties.contains("access_token");
	}

	bool google_account_info::has_login_data() const
	{
		return properties.contains("client_secret") and properties.contains("client_id"); 
	}

	bool google_account_info::has_refresh_token() const
	{
		return properties.contains("refresh_token");
	}

	google_account_manager::google_account_manager() :
		service_account_manager(static_service_id, static_service_display_name)
	{
	}

	nlohmann::ordered_json google_account_manager::save() const
	{
		auto json_account = nlohmann::ordered_json::object();
		
		auto status = login_status();

		//TODO: improve / handle errors
		if (status != account_login_status::not_logged_in)
		{
			json_account["user_name"] = account_info_.user_name();
			json_account["client_id"] = account_info_.client_id();
			json_account["client_secret"] = account_info_.client_secret();
			json_account["refresh_token"] = account_info_.refresh_token();
		}

		return json_account;
	}

	void google_account_manager::load(const nlohmann::ordered_json& json)
	{
		account_properties properties;

		if (!json.contains("user_name") or !json.contains("client_id") or !json.contains("client_secret") or !json.contains("refresh_token"))
		{
			return;
		}

		properties["client_id"] = json.at("client_id");
		properties["client_secret"] = json.at("client_secret");
		properties["refresh_token"] = json.at("refresh_token");
		properties["user_name"] = json.at("user_name");

		auto result = refresh_access_token(properties.at("client_id"), properties.at("client_secret"), properties.at("refresh_token"));
		if (!result.has_value())
		{
			debug::error("Failed to refresh access token for service {}", service_id());
			account_info_.properties = properties;
			return;
		}

		properties["access_token"] = std::move(result->access_token);
		properties["scope"] = std::move(result->scope);

		account_info_.expire_tp = result->expire_tp;
		account_info_.properties = properties;
	}

	std::string google_account_manager::account_name() const
	{
		return account_info_.user_name();
	}

	bool google_account_manager::on_log_in(const account_properties& properties, bool* cancel_token)
	{
		//TODO: if refresh token is present obtain token from it

		std::string client_id = properties.at("client_id");
		std::string client_secret = properties.at("client_secret");
		std::string refresh_token;
		std::string access_token;
		std::vector<std::string> scope;
		std::chrono::steady_clock::time_point expire_tp;
			
		bool refresh_success = false;
		if (properties.contains("refresh_token"))
		{
			auto result = refresh_access_token(client_id, client_secret, properties.at("refresh_token"));
			if (result.has_value())
			{
				refresh_success = true;
				refresh_token = properties.at("refresh_token");
				access_token = std::move(result->access_token);
				scope = std::move(result->scope);
				expire_tp = result->expire_tp;
			}
		}

		if (!refresh_success)
		{
			auto result = obtain_access_token(client_id, client_secret, cancel_token);
			if (!result.has_value())
			{
				debug::error("Failed to obtain access token");
				return false;
			}

			refresh_token = std::move(result->refresh_token);
			access_token = std::move(result->access_token);
			scope = std::move(result->scope);
			expire_tp = result->expire_tp;
		}

		account_info_.properties["client_id"] = client_id;
		account_info_.properties["client_secret"] = client_secret;

		account_info_.properties["access_token"] = access_token;
		account_info_.properties["refresh_token"] = refresh_token;
		account_info_.properties["scope"] = scope;
		account_info_.expire_tp = expire_tp;

		auto user_name = obtain_user_name();
		account_info_.properties["user_name"] = user_name.value_or("unknown");

		return true;
	}

	bool google_account_manager::on_retry_login()
	{
		if (!account_info_.has_login_data() or !account_info_.has_refresh_token())
		{
			return false;
		}

		std::string client_id = account_info_.client_id();
		std::string client_secret = account_info_.client_secret();
		std::string refresh_token = account_info_.refresh_token();

		auto result = refresh_access_token(client_id, client_secret, refresh_token);
		if (!result.has_value())
		{
			debug::error("Failed to refresh access token for service {}", service_id());
			return false;
		}

		account_info_.properties["access_token"] = result->access_token;
		account_info_.properties["refresh_token"] = result->refresh_token;
		account_info_.expire_tp = result->expire_tp;

		return true;
	}

	void google_account_manager::on_log_out()
	{
		if (account_info_.has_access_token())
		{
			revoke_token();
		}
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

	account_login_status google_account_manager::login_status() const
	{
		if (account_info_.has_access_token())
		{
			return account_login_status::logged_in;
		}
		if (account_info_.has_access_token() and account_info_.access_token_expired())
		{
			return account_login_status::expired;
		}
		if (account_info_.has_login_data() and account_info_.has_refresh_token() and !account_info_.has_access_token())
		{
			return account_login_status::refresh_failed;
		}

		return account_login_status::not_logged_in;
	}

	bool google_account_manager::draw_login_popup(bool& success)
	{
		//TODO: this needs improving

		static std::string client_id;
		static std::string client_secret;
		static bool cancel_token = false;

		bool return_value = false;

		bool open_login_failed_popup = false;

		if (ImGui::BeginPopupModal("Waiting", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize))
		{
			ImGui::TextUnformatted("Waiting for login...");
			
			if (add_account_result_.wait_for(std::chrono::seconds{ 0 }) == std::future_status::ready)
			{
				success = add_account_result_.get();
				if (success)
				{
					return_value = true;
					ImGui::CloseCurrentPopup();
				}
				else
				{
					open_login_failed_popup = true;
					ImGui::CloseCurrentPopup();
				}
			}
		
			if (ImGui::Button("Cancel"))
			{
				cancel_token = true;
				add_account_result_.get();
				success = false;
				return_value = true;
				ImGui::CloseCurrentPopup();
			}
		
			ImGui::EndPopup();
		}

		if (open_login_failed_popup)
		{
			ImGui::OpenPopup("Login failed");
		}

		if (ImGui::BeginPopupModal("Login failed", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize))
		{
			ImGui::TextUnformatted("Login attempt failed");
			if (ImGui::Button("OK"))
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		ImGui::InputText("Client id", &client_id);
		ImGui::InputText("Client secret", &client_secret);
		
		if (ImGui::Button("Log in"))
		{
			account_properties properties;
			properties["client_id"] = client_id;
			properties["client_secret"] = client_secret;
			cancel_token = false;
			add_account_result_ = log_in(properties, &cancel_token);
			ImGui::OpenPopup("Waiting");
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			client_id.clear();
			client_secret.clear();

			success = false;
			return_value = true;
		}

		return return_value;
	}

	std::optional<obtain_token_result> google_account_manager::obtain_access_token(const std::string& client_id, const std::string& client_secret, bool* cancel_token)
	{
		std::string code_verifier = utils::oauth2::generate_code_verifier();
		std::string code_challenge = utils::oauth2::generate_code_challenge(code_verifier);

		httplib::Server server;

		int redirect_port = 6060;

		std::string get_path = fmt::format("/oauth2/{}", utils::random::get<uint64_t>());
		std::string redirect_uri = fmt::format("http://127.0.0.1:{}{}", redirect_port, get_path);

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
				client_id, redirect_uri, fmt::join(request_scope_, " "), code_challenge
			);

			std::string auth_url = auth_host + auth_get;
			SDL_OpenURL(auth_url.c_str());

			server.Get(get_path, [&oauth2_code, &server](const httplib::Request& req, httplib::Response& res)
			{
				if (req.params.count("code") != 0)
				{
					res.set_content(service_account_manager::success_page(), "text/html");
					oauth2_code = req.params.find("code")->second;
				}
				else
				{
					res.set_content(service_account_manager::failure_page(req.params.find("error")->second), "text/html");
				}
				
				server.stop();
			});

			//TODO: launch the server on a separate thread so it can be stopped
			auto future = std::async(std::launch::async, [&]()
			{
				server.listen("127.0.0.1", redirect_port);
			});

			while (!(future.wait_for(std::chrono::seconds{ 0 }) == std::future_status::ready))
			{
				if (cancel_token != nullptr and *cancel_token)
				{
					server.stop();
					break;
				}
			}
			future.wait();
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
		auto post_result = client.Post("/token", params);
		if (!post_result)
		{
			debug::error("POST failed: {}", httplib::to_string(post_result.error()));
			return std::nullopt;
		}
		if (post_result->status != 200)
		{
			debug::error("Got response: {} {}", post_result->status, post_result->reason);
			return std::nullopt;
		}

		obtain_token_result result;

		//TODO: handle errors?
		auto json = nlohmann::json::parse(post_result->body);
		result.access_token = json.at("access_token");
		result.refresh_token = json.at("refresh_token");
		result.expire_tp = std::chrono::steady_clock::now() + std::chrono::seconds{ int(json.at("expires_in")) };
		result.scope = utils::string::split(json.at("scope"), ' ');
		
		std::vector<std::string> required_scopes(request_scope_.begin(), request_scope_.end());

		std::sort(result.scope.begin(), result.scope.end());
		std::sort(required_scopes.begin(), required_scopes.end());

		if (required_scopes != result.scope)
		{
			debug::error("User didn't grant all the required scopes");
			return std::nullopt;
		}

		return result;
	}

	std::optional<obtain_token_result> google_account_manager::refresh_access_token(const std::string& client_id, const std::string& client_secret, const std::string& refresh_token)
	{
		httplib::Client client("https://oauth2.googleapis.com");
		httplib::Params params
		{
			{ "client_id", client_id },
			{ "client_secret", client_secret },
			{ "grant_type", "refresh_token" },
			{ "refresh_token", refresh_token }
		};
		auto post_result = client.Post("/token", params);
		if (!post_result)
		{
			debug::error("POST failed: {}", httplib::to_string(post_result.error()));
			return std::nullopt;
		}
		if (post_result->status != 200)
		{
			debug::error("Got response: {} {}", post_result->status, post_result->reason);
			return std::nullopt;
		}

		obtain_token_result result;

		//TODO: handle errors?
		auto json = nlohmann::json::parse(post_result->body);
		result.access_token = json.at("access_token");
		result.expire_tp = std::chrono::steady_clock::now() + std::chrono::seconds{ int(json.at("expires_in")) };
		result.scope = utils::string::split(json.at("scope"), ' ');

		return result;
	}

	std::optional<std::string> google_account_manager::access_token()
	{
		if (account_info_.access_token_expired())
		{
			auto result = refresh_access_token(account_info_.client_id(), account_info_.client_secret(), account_info_.refresh_token());
			if (!result.has_value())
			{
				debug::error("Failed to refresh access token for service {}", service_id());
				return std::nullopt;
			}

			account_info_.properties["access_token"] = std::move(result->access_token);
			account_info_.properties["scope"] = std::move(result->scope);

			account_info_.expire_tp = result->expire_tp;
		}

		if (!account_info_.properties.contains("access_token"))
		{
			return std::nullopt;
		}

		return account_info_.access_token();
	}

	std::optional<std::string> google_account_manager::obtain_user_name()
	{
		httplib::Client client("https://www.googleapis.com");
		client.set_bearer_token_auth(*access_token());
		auto get_result = client.Get("/oauth2/v3/userinfo");
		if (!get_result)
		{
			debug::error("GET failed: {}", httplib::to_string(get_result.error()));
			return std::nullopt;
		}
		if (get_result->status != 200)
		{
			debug::error("Got response: {} {}", get_result->status, get_result->reason);
			return std::nullopt;
		}

		auto json = nlohmann::json::parse(get_result->body);
		return json.at("name");
	}

	bool google_account_manager::revoke_token()
	{
		if (!account_info_.properties.contains("access_token"))
		{
			return false;
		}

		httplib::Client client("https://oauth2.googleapis.com");
		httplib::Params params
		{
			{ "token", account_info_.access_token() }
		};
		auto post_result = client.Post("/revoke", params);
		if (!post_result)
		{
			debug::error("POST failed: {}", httplib::to_string(post_result.error()));
			return false;
		}
		if (post_result->status != 200)
		{
			debug::error("Got response: {} {}", post_result->status, post_result->reason);
			return false;
		}

		return true;
	}
}
