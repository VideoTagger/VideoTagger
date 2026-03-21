#include "github_source.hpp"

#include <algorithm>
#include <fstream>
#include <httplib.h>
#include <utils/json.hpp>

namespace vt
{
	github_source::github_source(const std::string& url, const std::optional<std::string>& access_token, bool prerelease) :
		url_{ url }, access_token_{ access_token }, prerelease_{ prerelease } {}
	
	const std::string github_source::GetReleaseFeed(const std::string release_name)
	{
		auto releases = get_releases();
		for (const auto& release : releases)
		{
			std::string url = get_asset_url_from_name(release, release_name);
			if (!url.empty())
			{
				return download_url_as_string_with_headers(url, "application/octet-stream");
			}
		}
		return "";
	}

	bool github_source::DownloadReleaseEntry(const Velopack::VelopackAsset& asset, const std::string local_path, Velopack::vpkc_progress_send_t progress)
	{
		auto releases = get_releases();
		std::string target_url;
		for (const auto& release : releases)
		{
			target_url = get_asset_url_from_name(release, asset.FileName);
			if (!target_url.empty())
			{
				break;
			}
		}

		if (target_url.empty())
		{
			return false;
		}

		std::string client_path;
		std::string client_base;

		auto pos = target_url.find("://");
		std::string scheme = "https://";
		std::string rest = target_url;
		if (pos != std::string::npos)
		{
			scheme = target_url.substr(0, pos + 3);
			rest = target_url.substr(pos + 3);
		}
		auto path_pos = rest.find('/');
		if (path_pos != std::string::npos)
		{
			client_base = scheme + rest.substr(0, path_pos);
			client_path = rest.substr(path_pos);
		}
		else
		{
			client_base = scheme + rest;
			client_path = "/";
		}

		httplib::Client cli(client_base);
		cli.set_follow_location(true);

		httplib::Headers headers
		{
			{ "Accept", "application/octet-stream" }
		};
		if (access_token_)
		{
			headers.insert({ "Authorization", "Bearer " + *access_token_ });
		}

		std::ofstream out(local_path, std::ios::binary);
		if (!out.is_open()) return false;

		auto res = cli.Get(client_path, headers,
		[&](const char* data, size_t data_length)
		{
			out.write(data, data_length);
			return true;
		},
		[&](uint64_t len, uint64_t total)
		{
			if (progress and total > 0)
			{
				int16_t pc = static_cast<int16_t>((len * 100) / total);
				progress(pc);
			}
			return true;
		});
		return res and res->status == 200;
	}

	std::vector<github_release> github_source::get_releases() const
	{
		std::string url = url_;
		while (!url.empty() and url.back() == '/')
		{
			url.pop_back();
		}

		std::string scheme = "https";
		auto scheme_pos = url.find("://");
		if (scheme_pos != std::string::npos)
		{
			scheme = url.substr(0, scheme_pos);
			url = url.substr(scheme_pos + 3);
		}

		auto path_pos = url.find('/');
		std::string host = url;
		std::string repo_path;
		if (path_pos != std::string::npos)
		{
			host = url.substr(0, path_pos);
			repo_path = url.substr(path_pos);
		}

		std::string lower_host = host;
		std::transform(lower_host.begin(), lower_host.end(), lower_host.begin(), [](unsigned char c)
		{
			return std::tolower(c);
		});

		std::string base;
		std::string client_path;
		if (lower_host == "github.com")
		{
			base = scheme + "://api.github.com";
			client_path = "/repos" + repo_path + "/releases?per_page=10&page=1";
		}
		else
		{
			base = scheme + "://" + host;
			client_path = "/api/v3/repos" + repo_path + "/releases?per_page=10&page=1";
		}

		httplib::Client cli(base);
		cli.set_follow_location(true);

		httplib::Headers headers
		{
			{ "Accept", "application/vnd.github.v3+json" }
		};

		if (access_token_.has_value())
		{
			headers.insert({ "Authorization", "Bearer " + access_token_.value() });
		}

		auto res = cli.Get(client_path, headers);
		if (!res or res->status != 200)
		{
			return {};
		}

		auto json = nlohmann::json::parse(res->body, nullptr, false);
		if (json.is_discarded() or !json.is_array())
		{
			return {};
		}

		std::vector<github_release> releases;
		for (const auto& item : json)
		{
			github_release release;
			release.name = item.value("name", "");
			release.prerelease = item.value("prerelease", false);
			release.published_at = item.value("published_at", "");

			if (item.contains("assets") and item["assets"].is_array())
			{
				for (const auto& asset : item["assets"])
				{
					github_release_asset rel_asset;
					rel_asset.name = asset.value("name", "");
					rel_asset.url = asset.value("url", "");
					rel_asset.browser_download_url = asset.value("browser_download_url", "");
					release.assets.push_back(rel_asset);
				}
			}
			releases.push_back(release);
		}

		std::sort(releases.begin(), releases.end(), [](const github_release& left, const github_release& right)
		{
			return right.published_at < left.published_at;
		});

		if (!prerelease_)
		{
			auto it = std::remove_if(releases.begin(), releases.end(), [](const github_release& release)
			{
				return release.prerelease;
			});
			releases.erase(it, releases.end());
		}

		return releases;
	}

	std::string github_source::get_asset_url_from_name(const github_release& release, const std::string& asset_name) const
	{
		for (const auto& asset : release.assets)
		{
			std::string lower_name = asset.name;
			std::string lower_asset_name = asset_name;

			std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), [](unsigned char c)
			{
				return std::tolower(c);
			});

			std::transform(lower_asset_name.begin(), lower_asset_name.end(), lower_asset_name.begin(), [](unsigned char c)
			{
				return std::tolower(c);
			});

			if (lower_name == lower_asset_name)
			{
				if (!access_token_)
				{
					if (!asset.browser_download_url.empty())
					{
						return asset.browser_download_url;
					}
				}
				if (!asset.url.empty())
				{
					return asset.url;
				}
			}
		}
		return "";
	}

	std::string github_source::download_url_as_string_with_headers(const std::string& url, const std::string& accept) const
	{
		std::string client_path;
		std::string client_base;

		auto pos = url.find("://");
		std::string scheme = "https://";
		std::string rest = url;

		if (pos != std::string::npos)
		{
			scheme = url.substr(0, pos + 3);
			rest = url.substr(pos + 3);
		}
		auto path_pos = rest.find('/');
		if (path_pos != std::string::npos)
		{
			client_base = scheme + rest.substr(0, path_pos);
			client_path = rest.substr(path_pos);
		}
		else
		{
			client_base = scheme + rest;
			client_path = "/";
		}

		httplib::Client cli(client_base);
		cli.set_follow_location(true);

		httplib::Headers headers
		{
			{ "Accept", accept }
		};

		if (access_token_)
		{
			headers.insert({ "Authorization", "Bearer " + *access_token_ });
		}

		auto res = cli.Get(client_path, headers);
		if (res and res->status == 200)
		{
			return res->body;
		}
		return "";
	}
}
