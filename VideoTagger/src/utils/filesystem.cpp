#include "pch.hpp"
#include "filesystem.hpp"

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
}
