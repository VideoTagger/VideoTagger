#pragma once
#include <string>
#include <optional>
#include "query.hpp"
#include <tags/tag.hpp>
#include <utils/vec.hpp>

namespace vt
{
	struct active_video_tex_size_query : public query<std::optional<utils::vec2<uint32_t>>>
	{
		active_video_tex_size_query() {}
	};

	struct active_video_tex_size_query_handler : public query_handler<active_video_tex_size_query>
	{
		std::optional<utils::vec2<uint32_t>> handle(const active_video_tex_size_query& query) override;
	};
}
