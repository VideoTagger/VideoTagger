#pragma once
#include <string>
#include <unordered_map>
#include "account_info.hpp"

namespace vt
{
	class service_account_manager
	{
	public:
		using container = std::unordered_map<std::string, account_info>;

		virtual const std::string& service_name() const = 0;

		virtual nlohmann::ordered_json save() const = 0;
		virtual void load(const nlohmann::ordered_json& json) = 0;

		bool add_account(const std::string& name, const account_info& info);
		bool remove_account(const std::string& name);

		bool contains_account(const std::string& name) const;

		const account_info& get_account_info(const std::string& name) const;
		account_info& get_account_info(const std::string& name);

		virtual void options_draw_account(const std::string& account_name) = 0;
		//return true if the popup is ready to be closed
		virtual bool add_popup_draw(bool& success) = 0;

		const container& accounts() const;

	private:
		container accounts_;
	};

	inline void to_json(nlohmann::ordered_json& json, const service_account_manager& manager)
	{
		json = manager.save();
	}

	inline void from_json(const nlohmann::ordered_json& json, service_account_manager& manager)
	{
		manager.load(json);
	}
}
