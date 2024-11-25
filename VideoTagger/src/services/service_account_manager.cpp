#include "pch.hpp"
#include "service_account_manager.hpp"

namespace vt
{
	service_account_manager::service_account_manager(std::string service_id, std::string service_display_name)
		: service_id_{ std::move(service_id) }, service_display_name_{ std::move(service_display_name) }
	{
	}

	const std::string& service_account_manager::service_id() const
	{
		return service_id_;
	}

	const std::string& service_account_manager::service_display_name() const
	{
		return service_display_name_;
	}

	std::future<bool> service_account_manager::log_in(const account_properties& properties, bool* cancel_token)
	{
		return std::async(std::launch::async, [this, cancel_token, properties]()
		{
			if (!on_log_in(properties, cancel_token))
			{
				return false;
			}
			return true;
		});
	}

	void service_account_manager::log_out()
	{
		on_log_out();
	}
}
