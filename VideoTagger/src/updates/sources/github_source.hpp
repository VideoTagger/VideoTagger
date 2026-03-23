#pragma once
#include <string>
#include <optional>
#include <vector>
#include <Velopack.hpp>

namespace vt
{
	struct github_release_asset
	{
		std::string browser_download_url;
		std::string url;
		std::string name;
	};

	struct github_release
	{
		std::vector<github_release_asset> assets;
		std::string name;
		std::string published_at;
		bool prerelease;
	};

	class github_source : public Velopack::IUpdateSource
	{
	public:
		github_source(const std::string& url, const std::optional<std::string>& access_token = std::nullopt, bool prerelease = false);

	private:
		std::string url_;
		std::optional<std::string> access_token_;
		bool prerelease_;

	public:
		virtual const std::string GetReleaseFeed(const std::string release_name) override;
		virtual bool DownloadReleaseEntry(const Velopack::VelopackAsset& asset, const std::string local_path, Velopack::vpkc_progress_send_t progress) override;
	private:
		std::vector<github_release> get_releases() const;
		std::string get_asset_url_from_name(const github_release& release, const std::string& asset_name) const;
		std::string download_url_as_string_with_headers(const std::string& url, const std::string& accept) const;
	};
}
