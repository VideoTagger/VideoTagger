#include "pch.hpp"
#include "google_account_manager.hpp"
#include <utils/oauth2.hpp>
#include <utils/string.hpp>
#include <core/debug.hpp>
#include <utils/random.hpp>
#include <core/app_context.hpp>
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

	std::string google_account_manager::service_id() const
	{
		return static_service_id;
	}

	std::string google_account_manager::service_display_name() const
	{
		return static_service_display_name;
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
			ctx_.console.add_entry(widgets::console::entry::flag_type::error, "Google account: failed to log in", widgets::console::entry::source_info{ "VideoTagger", -1 });
			return;
		}

		properties["access_token"] = std::move(result->access_token);
		properties["scope"] = std::move(result->scope);

		account_info_.expire_tp = result->expire_tp;
		account_info_.properties = properties;
	}

	account_properties google_account_manager::get_account_properties_from_file(const std::filesystem::path& file_path)
	{
		account_properties result;
		std::ifstream file(file_path);
		if (!file.is_open())
		{
			return result;
		}

		auto json = nlohmann::json::parse(file);

		if (json.contains("installed"))
		{
			json = json.at("installed");
		}

		if (json.contains("client_id"))
		{
			result["client_id"] = json.at("client_id");
		}
		if (json.contains("client_secret"))
		{
			result["client_secret"] = json.at("client_secret");
		}

		return result;
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

	account_login_popup_data google_account_manager::login_popup_data()
	{
		account_login_popup_data data;
		data.fields = {
			account_login_popup_field_data{ "Client id", "client_id" },
			account_login_popup_field_data{ "Client secret", "client_secret" }
		};

		data.show_file_load_button = true;

		return data;
	}

	std::optional<obtain_token_result> google_account_manager::obtain_access_token(const std::string& client_id, const std::string& client_secret, bool* cancel_token)
	{
		std::string code_verifier = utils::oauth2::generate_code_verifier();
		std::string code_challenge = utils::oauth2::generate_code_challenge(code_verifier);

		httplib::Server server;

		int redirect_port = 6060;

		std::string get_path = fmt::format("/oauth2/{}", utils::random::get<uint64_t>());
		std::string redirect_uri = fmt::format("http://127.0.0.1:{}{}", redirect_port, get_path);

		std::optional<obtain_token_result> result;
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


			server.Get(get_path, [&result, &server, client_id, client_secret, code_verifier, redirect_uri](const httplib::Request& req, httplib::Response& res)
			{
				std::string oauth2_code;

				auto on_fail = [&res, &req, &server](const std::string& reason)
				{
					res.set_content(service_account_manager::failure_page(reason), "text/html");
					server.stop();
				};

				if (req.params.count("code") == 0)
				{
					on_fail(req.params.find("error")->second);
					return;
				}

				oauth2_code = req.params.find("code")->second;
				
				httplib::Client client("https://oauth2.googleapis.com");
				httplib::Params params
				{
					{"client_id", client_id},
					{"client_secret", client_secret},
					{"code", oauth2_code},
					{"code_verifier", code_verifier},
					{"grant_type", "authorization_code"},
					{"redirect_uri", redirect_uri}
				};
				auto post_result = client.Post("/token", params);
				if (!post_result)
				{
					debug::error("POST failed: {}", httplib::to_string(post_result.error()));
					on_fail("Failed to obtain access token");
					return;
				}
				if (post_result->status != 200)
				{
					debug::error("Got response: {} {}", post_result->status, post_result->reason);
					on_fail("Failed to obtain access token");
					return;
				}

				//TODO: handle errors?
				result = obtain_token_result{};
				auto json = nlohmann::json::parse(post_result->body);
				result->access_token = json.at("access_token");
				result->refresh_token = json.at("refresh_token");
				result->expire_tp = std::chrono::steady_clock::now() + std::chrono::seconds{ int(json.at("expires_in")) };
				result->scope = utils::string::split(json.at("scope"), ' ');

				std::vector<std::string> required_scopes(request_scope_.begin(), request_scope_.end());

				std::sort(result->scope.begin(), result->scope.end());
				std::sort(required_scopes.begin(), required_scopes.end());

				if (required_scopes != result->scope)
				{
					debug::error("User didn't grant all the required scopes");
					result.reset();
					on_fail("Not all required permissions were granted");
					return;
				}

				res.set_content(service_account_manager::success_page(), "text/html");

				server.stop();
			});

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

		if (!result.has_value())
		{
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
