#pragma once
#include <uriparser/Uri.h>
#include <string>
#include <optional>

namespace vt::utils
{
	struct url
	{
	public:
		url();
		url(const url& other);
		url(url&& other) noexcept;
		~url();

	private:
		UriUriA uri_;

	public:
		///@returns The scheme of the URL (e.g., "http", "https", "ftp").
		std::string scheme() const;
		///@returns The user information of the URL (e.g., "user:password"), if specified, empty string otherwise.
		std::string user() const;
		///@returns The host of the URL (e.g., "example.com", "localhost", "
		std::string host() const;
		///@return The path of the URL (e.g., "/path/to/resource"), if specified, empty string otherwise.
		std::string path() const;
		///@returns The query of the URL (e.g., "key1=value1&key2=value2"), if specified, empty string otherwise.
		std::string query() const;
		///@returns The fragment of the URL after '#' (e.g., "section1"), if specified, empty string otherwise.
		std::string fragment() const;
		///@returns The port of the URL, if specified, std::nullopt otherwise.
		std::optional<uint16_t> port() const;

		///@returns The origin of the URL, which consists of the scheme, host, and the port.
		std::string origin() const;
		//@returns The relative path of the URL, which consists of the path, query, and fragment.
		std::string relative_path() const;

		std::string to_string() const;

		url& operator=(const url& other);
		url& operator=(url&& other) noexcept;

		static std::optional<url> from_string(const std::string& url_str);
	private:
		static std::string extract_uri_range(const UriTextRangeA& range);
	};
}
