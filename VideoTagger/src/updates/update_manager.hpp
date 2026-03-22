#pragma once
#include <memory>
#include <optional>
#include <Velopack.hpp>

namespace vt
{
	struct update_info
	{
		Velopack::UpdateInfo vpk_info;
	};

	struct update_manager
	{
	public:
		update_manager() = delete;

	private:
		static std::unique_ptr<Velopack::UpdateManager> manager_;

	public:
		static void init();
		static void shutdown();
		static void update(const update_info& info);

		static std::optional<update_info> check_for_updates();
	};
}
