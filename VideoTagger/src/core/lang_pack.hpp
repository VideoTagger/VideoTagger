#pragma once
#include <string>
#include <unordered_map>

namespace vt
{
	template<typename lang_pack_id_t>
	using lang_pack_data = std::unordered_map<lang_pack_id_t, std::string>;

	template<typename lang_pack_id_t>
	class lang_pack
	{
	public:
		constexpr lang_pack() = default;
		constexpr lang_pack(const lang_pack_data<lang_pack_id_t>& data) : data_{ data } {}

	private:
		lang_pack_data<lang_pack_id_t> data_;

	public:
		std::string& operator[](lang_pack_id_t id)
		{
			return data_[id];
		}

		const std::string& operator[](lang_pack_id_t id) const
		{
			return data_.at(id);
		}

		void replace_data(const lang_pack_data<lang_pack_id_t>& data)
		{
			data_ = data;
		}

		const char* get(lang_pack_id_t id) const
		{
			auto it = data_.find(id);
			if (it != data_.end()) return it->second.c_str();
			return "<Missing>";
		}
	};
}
