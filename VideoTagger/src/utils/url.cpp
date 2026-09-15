#include "url.hpp"
#include <fmt/format.h>

namespace vt::utils
{
	url::url() : uri_{} {}

	url::url(const url& other) : url()
	{
		if (uriCopyUriA(&uri_, &other.uri_) != URI_SUCCESS)
		{
			throw std::runtime_error("Failed to copy URL");
		}
	}

	url::url(url&& other) noexcept : uri_{ std::move(other.uri_) }
	{
		other.uri_ = {};
	}

	url::~url()
	{
		uriFreeUriMembersA(&uri_);
	}


	std::string url::scheme() const
	{
		return extract_uri_range(uri_.scheme);
	}

	std::string url::user() const
	{
		return extract_uri_range(uri_.userInfo);
	}
	
	std::string url::host() const
	{
		return extract_uri_range(uri_.hostText);
	}

	std::string url::path() const
	{
		if (uri_.pathHead == nullptr) return {};

		std::string result;
		UriPathSegmentA* current = uri_.pathHead;
		while (current)
		{
			result += "/" + extract_uri_range(current->text);
			current = current->next;
		}
		return result;
	}
	
	std::string url::query() const
	{
		return extract_uri_range(uri_.query);
	}

	std::string url::fragment() const
	{
		return extract_uri_range(uri_.fragment);
	}

	std::optional<uint16_t> url::port() const
	{
		auto port_str = extract_uri_range(uri_.portText);
		if (port_str.empty()) return std::nullopt;
		return std::optional<uint16_t>(static_cast<uint16_t>(std::stoi(port_str)));
	}

	std::string url::origin() const
	{
		auto usr = user();
		auto prt = port();
		return fmt::format("{}://{}{}{}", scheme(), !usr.empty() ? fmt::format("{}@", usr) : "", host(), prt.has_value() ? fmt::format(":{}", prt.value()) : "");
	}

	std::string url::relative_path() const
	{
		auto qry = query();
		auto frag = fragment();
		return fmt::format("{}{}{}", path(), !qry.empty() ? fmt::format("?{}", qry) : "", !frag.empty() ? fmt::format("#{}", frag) : "");
	}

	std::string url::to_string() const
	{
		int chars_required = 0;
		if (uriToStringCharsRequiredA(&uri_, &chars_required) != URI_SUCCESS) return {};

		int chars_written = 0;
		std::string buffer(chars_required + 1, '\0');
		if (uriToStringA(&buffer[0], &uri_, chars_required + 1, &chars_written) != URI_SUCCESS) return {};
		buffer.resize(chars_required);
		return buffer;
	}

	url& url::operator=(const url& other)
	{
		if (this != &other)
		{
			uriFreeUriMembersA(&uri_);
			std::memset(&uri_, 0, sizeof(UriUriA));
			if (uriCopyUriA(&uri_, &other.uri_) != URI_SUCCESS)
			{
				throw std::runtime_error("Failed to copy URL");
			}
		}
		return *this;
	}

	url& url::operator=(url&& other) noexcept
	{
		if (this != &other)
		{
			uriFreeUriMembersA(&uri_);
			uri_ = std::move(other.uri_);
			other.uri_ = {};
		}
		return *this;
	}

	std::optional<url> url::from_string(const std::string& url_str)
	{
		const char* error_pos = nullptr;

		url result;
		if (uriParseSingleUriA(&result.uri_, url_str.c_str(), &error_pos) != URI_SUCCESS)
		{
			uriFreeUriMembersA(&result.uri_);
			return std::nullopt;
		}

		return result;
	}

	std::string url::extract_uri_range(const UriTextRangeA& range)
	{
		if (range.first != nullptr and range.afterLast != nullptr and range.afterLast >= range.first)
		{
			return std::string{ range.first, range.afterLast };
		}
		return {};
	}
}
