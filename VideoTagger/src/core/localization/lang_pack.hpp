#pragma once
#include <string>
#include <string_view>
#include <filesystem>
#include <optional>
#include <unordered_map>
#include <fmt/format.h>

#include <utils/json.hpp>

namespace vt
{
	using lang_pack_data = std::unordered_map<std::string, std::string>;

	struct lang_template
	{
		static constexpr std::string_view prefix = "template";

		size_t param_count{};
		std::string id{};
	};

	class lang_pack
	{
	public:
		static constexpr const char* extension = "vtlang";

		lang_pack(const std::string& name, const std::string& filename, const lang_pack_data& data = {}, bool editable = true);

	private:
		std::string filename_;
		std::string name_;
		bool editable_;
		bool is_dirty_;
		lang_pack_data data_;

	public:
		void set_dirty(bool value);

		std::string& name();
		const std::string& name() const;
		std::string& filename();
		const std::string& filename() const;
		std::vector<std::string> keys() const;
		bool is_dirty() const;

		std::string get(const std::string& id);
		std::string& at(const std::string& id);
		const std::string& at(const std::string& id) const;
		std::string& operator[](const std::string& id);
		const std::string& operator[](const std::string& id) const;

		void save(const std::filesystem::path& dir);

		static std::optional<lang_pack> load_from_json(const nlohmann::ordered_json& json, const std::string& filename);
		static std::optional<lang_pack> load_from_file(const std::filesystem::path& path);

		bool is_template(const std::string& id) const;
		///@return Parsed template if the template with the given id exists and is valid, std::nullopt otherwise
		std::optional<lang_template> parse_template(const std::string& id) const;

		///@return True if the template is valid, false otherwise. If the template is valid, the parsed template is stored in target
		bool try_parse_template(const std::string& id, lang_template& target) const;
		template<typename... arguments>
		std::string get_template(const std::string& id, arguments&&... args)
		{
			constexpr auto arg_count = sizeof...(arguments);
			return fmt::format(fmt::runtime(get(fmt::format("template:{}:{}", arg_count, id))), std::forward<arguments>(args)...);
		}
	};
}
