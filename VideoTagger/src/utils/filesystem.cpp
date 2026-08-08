#include "pch.hpp"
#include "string.hpp"
#include "filesystem.hpp"
#include <core/platform.hpp>
#include <utils/url.hpp>


#ifdef VT_OS_WINDOWS
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
	#include <shlobj.h>
	#include <shobjidl.h>
	#include <windows.h>
#endif

static nfdu8char_t* make_nfd_path(const std::string& input)
{
	return (nfdu8char_t*)input.c_str();
}

static std::filesystem::path convert_nfd_path(const NFD::UniquePathU8& input)
{
	auto ptr = input.get();
	return ptr != nullptr ? std::filesystem::path{ (const char*)ptr } : std::filesystem::path{};
}

static std::vector<std::filesystem::path> convert_nfd_paths(const NFD::UniquePathSet& input)
{
	std::vector<std::filesystem::path> result;
	auto ptr = input.get();

	if (ptr != nullptr)
	{
		nfdpathsetsize_t size = 0;
		NFD::PathSet::Count(input.get(), size);
		for (nfdpathsetsize_t i = 0; i < size; ++i)
		{
			NFD::UniquePathSetPathU8 path;
			NFD::PathSet::GetPath(input, i, path);
			result.push_back(convert_nfd_path(*((NFD::UniquePathU8*)&path)));
		}
	}
	return result;
}

static std::vector<nfdu8filteritem_t> make_nfd_filters(const vt::utils::dialog_filters& input)
{
	std::vector<nfdu8filteritem_t> result;
	for (const auto& filter : input)
	{
		result.push_back({ filter.description.c_str(), filter.extensions.c_str() });
	}
	return result;
}

static vt::utils::dialog_option convert_nfd_option(nfdresult_t nfd_option)
{
	vt::utils::dialog_option option{ vt::utils::dialog_option::error };
	switch (nfd_option)
	{
		case nfdresult_t::NFD_OKAY: option = vt::utils::dialog_option::ok; break;
		case nfdresult_t::NFD_CANCEL: option = vt::utils::dialog_option::cancel; break;
	}
	return option;
}

namespace vt::utils
{
	dialog_result filesystem::get_file(const std::filesystem::path& start_dir, const dialog_filters& filters)
	{
		NFD::UniquePathU8 path;
		std::string dir_str = start_dir.string();
		auto nfd_result = NFD::OpenDialog(path, make_nfd_filters(filters).data(), (nfdfiltersize_t)filters.size(), make_nfd_path(dir_str));
		return { convert_nfd_option(nfd_result), convert_nfd_path(path) };
	}

	dialog_result filesystem::get_folder(const std::filesystem::path& start_dir)
	{
		NFD::UniquePathU8 path;
		std::string dir_str = start_dir.string();
		auto nfd_result = NFD::PickFolder(path, make_nfd_path(dir_str));
		return { convert_nfd_option(nfd_result), convert_nfd_path(path) };
	}

	dialog_result filesystem::save_file(const std::filesystem::path& start_dir, const dialog_filters& filters, const std::string& default_filename)
	{
		NFD::UniquePathU8 path;
		std::string dir_str = start_dir.string();
		auto nfd_result = NFD::SaveDialog(path, make_nfd_filters(filters).data(), (nfdfiltersize_t)filters.size(), make_nfd_path(dir_str), make_nfd_path(default_filename));
		return { convert_nfd_option(nfd_result), convert_nfd_path(path) };
	}

	dialog_results filesystem::get_files(const std::filesystem::path& start_dir, const dialog_filters& filters)
	{
		NFD::UniquePathSet paths;
		std::string dir_str = start_dir.string();
		auto nfd_result = NFD::OpenDialogMultiple(paths, make_nfd_filters(filters).data(), (nfdfiltersize_t)filters.size(), make_nfd_path(dir_str));
		return { convert_nfd_option(nfd_result), convert_nfd_paths(paths) };
	}

	std::string filesystem::normalize(const std::filesystem::path& filepath)
	{
		std::string result = filepath.u8string();
#ifdef _WIN32
		for (auto& c : result)
		{
			if (c == '\\') c = '/';
		}
#endif
		return result;
	}

	void filesystem::open_in_explorer(const std::filesystem::path& path)
	{
		std::string uri = fmt::format("file://{}", path.u8string());
		std::thread thread([uri]()
		{
			SDL_OpenURL(uri.c_str());
		});
		thread.detach();
	}

    void filesystem::open_file_in_explorer(const std::filesystem::path& path)
    {
#ifdef VT_OS_WINDOWS
		PIDLIST_ABSOLUTE pidl_folder = nullptr;

		auto result = SHParseDisplayName(path.c_str(), nullptr, &pidl_folder, 0, nullptr);
		if (!SUCCEEDED(result))
		{
			open_in_explorer(std::filesystem::absolute(path.parent_path()));
			return;
		}

		PCUITEMID_CHILD pidl_item = ILFindLastID(pidl_folder);
		PIDLIST_ABSOLUTE pidl_parent = ILClone(pidl_folder);
		ILRemoveLastID(pidl_parent);

		SHOpenFolderAndSelectItems(pidl_parent, 1, &pidl_item, 0);
		CoTaskMemFree(pidl_folder);
		CoTaskMemFree(pidl_parent);
#else
		open_in_explorer(std::filesystem::absolute(path.parent_path()));
#endif
	}

	std::string filesystem::concat_extensions(const std::vector<std::string>& extensions)
	{
		std::string result;
		for (size_t i = 0; i < extensions.size(); ++i)
		{
			result += extensions[i];
			if (i + 1 < extensions.size())
			{
				result += ',';
			}
		}
		return result;
	}
	std::filesystem::path filesystem::get_storage_path(const std::string& organization, const std::string& app_name)
	{
		auto buf = SDL_GetPrefPath(organization.c_str(), app_name.c_str());
		if (buf == nullptr) return {};

		std::string result = buf;
		SDL_free(buf);
		return result;
	}

	bool filesystem::is_subdirectory(const std::filesystem::path& parent, const std::filesystem::path& child)
	{
		auto parent_abs = std::filesystem::absolute(parent);
		auto child_abs = std::filesystem::absolute(child);

		auto parent_it = parent_abs.begin();
		for (auto child_it = child_abs.begin(); parent_it != parent_abs.end() and child_it != child_abs.end(); ++parent_it, ++child_it)
		{
			if (*parent_it != *child_it)
			{
				return false;
			}
		}

		return parent_it == parent_abs.end();
	}

	bool filesystem::download_file(const std::string& url, const std::filesystem::path& destination, std::optional<httplib::Headers> headers,
		std::optional<cancellation_token> cancel_token, const std::function<void(uint64_t current_size, uint64_t total_size, std::optional<cancellation_token> cancel_token)>& callback)
	{
		auto parsed_url = vt::utils::url::from_string(url);
		if (!parsed_url.has_value())
		{
			return false;
		}

		std::string url_host = parsed_url->origin();
		std::string url_path = parsed_url->relative_path();

		httplib::Client client(url_host);
		client.set_follow_location(true);
		client.set_keep_alive(true);
		client.set_default_headers(headers.value_or(httplib::Headers{}));

		auto parent_path = destination.parent_path();
		if (!parent_path.empty())
		{
			std::filesystem::create_directories(parent_path);
		}

		std::ofstream file(destination, std::ios::binary);
		if (!file.is_open())
		{
			return false;
		}

		auto download_progress_callback = [&cancel_token, &callback](uint64_t current_size, uint64_t total_size)
		{
			if (callback != nullptr)
			{
				callback(current_size, total_size, cancel_token);
			}
			if (cancel_token.has_value() and cancel_token->is_cancelled())
			{
				return false;
			}
			return true;
		};

		auto content_receiver_callback = [&file](const char* data, size_t data_length)
		{
			return false;
		};
		return client.Get(url_path, content_receiver_callback, download_progress_callback);
	}
}
