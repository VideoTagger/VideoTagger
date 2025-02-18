#include "pch.hpp"
#include "lang_pack.hpp"
#include <fmt/format.h>
#include <utils/json.hpp>

namespace vt
{
	lang_pack::lang_pack(const std::string& name, const std::string& filename, const lang_pack_data& data, bool editable) : name_{ name }, filename_{ filename }, data_{ data }, editable_{ editable }, is_dirty_{} {}

	void lang_pack::set_dirty(bool value)
	{
		is_dirty_ = value;
	}

	std::string& lang_pack::name()
	{
		return name_;
	}

	const std::string& lang_pack::name() const
	{
		return name_;
	}

	std::vector<std::string> lang_pack::keys() const
	{
		std::vector<std::string> result;
		result.reserve(data_.size());
		for (const auto& [key, _] : data_)
		{
			result.push_back(key);
		}
		return result;
	}

	std::string lang_pack::get(const std::string& id)
	{
		auto it = data_.find(id);
		if (it != data_.end() and !it->second.empty()) return it->second;
#ifdef _DEBUG
		data_.emplace(id, "");
#endif
		return fmt::format("<{}>", id);
	}

	std::string& lang_pack::at(const std::string& id)
	{
		return data_[id];
	}

	const std::string& lang_pack::at(const std::string& id) const
	{
		return data_.at(id);
	}

	std::string& lang_pack::operator[](const std::string& id)
	{
		return at(id);
	}

	const std::string& lang_pack::operator[](const std::string& id) const
	{
		return at(id);
	}

	std::optional<lang_pack> lang_pack::load_from_file(const std::filesystem::path& path)
	{
		auto json = utils::json::load_from_file(path);
		if (!json.contains("@meta")) return std::nullopt;

		auto json_info = json.at("@meta");
		if (!json_info.contains("name")) return std::nullopt;

		lang_pack lang(json_info.at("name"), path.stem().u8string(), {}, json_info.at("editable"));
		for (const auto& [key, value] : json.items())
		{
			if (key == "@meta" or !value.is_string()) continue;
			lang[key] = value.get<std::string>();
		}
		return lang;
	}
}
