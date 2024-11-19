#include "pch.hpp"
#include "service_account_manager.hpp"

namespace vt
{
	service_account_manager::service_account_manager(std::string service_id, std::optional<std::string> account_name)
		: service_id_{ std::move(service_id) }, account_name_{ std::move(account_name) }
	{
	}

	const std::string& service_account_manager::service_id() const
	{
		return service_id_;
	}

	const std::string& service_account_manager::account_name() const
	{
		return account_name_ ? *account_name_ : empty_account_name;
	}

	bool service_account_manager::add_account(const std::string& name, const account_properties& properties)
	{
		if (!on_add_account(name, properties))
		{
			return false;
		}

		account_name_ = name;
		return true;
	}

	void service_account_manager::remove_account()
	{
		on_remove_account();
		account_name_.reset();
	}

	bool service_account_manager::logged_in() const
	{
		return account_name_.has_value();
	}
}
