#pragma once
#include <string>
#include <filesystem>
#include <optional>
#include <unordered_map>

namespace vt
{
	using lang_pack_data = std::unordered_map<std::string, std::string>;

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

		static std::optional<lang_pack> load_from_file(const std::filesystem::path& path);
	};
}
