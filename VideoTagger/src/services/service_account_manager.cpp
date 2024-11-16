#include "pch.hpp"
#include "service_account_manager.hpp"

namespace vt
{
	bool service_account_manager::add_account(const std::string& name, const account_info& info)
	{
		return accounts_.try_emplace(name, info).second;
	}

	bool service_account_manager::remove_account(const std::string& name)
	{
		return accounts_.erase(name);
	}

	bool service_account_manager::contains_account(const std::string& name) const
	{
		return accounts_.find(name) != accounts_.end();

	}

	const account_info& service_account_manager::get_account_info(const std::string& name) const
	{
		return accounts_.at(name);
	}

	account_info& service_account_manager::get_account_info(const std::string& name)
	{
		return accounts_.at(name);
	}

	const std::unordered_map<std::string, account_info>& service_account_manager::accounts() const
	{
		return accounts_;
	}
}
