#include "pch.hpp"
#include "active_video_tex_size_query.hpp"
#include <core/app_context.hpp>

namespace vt
{
	std::optional<utils::vec2<uint32_t>> active_video_tex_size_query_handler::handle(const active_video_tex_size_query& query)
	{
		auto focused_id = ctx_.last_focused_video;
		if (!focused_id.has_value()) return std::nullopt;

		auto it = ctx_.displayed_videos.find(focused_id.value());
		if (it == ctx_.displayed_videos.end()) return std::nullopt;
		return utils::vec2<uint32_t>{ (uint32_t)it->display_texture.width(), (uint32_t)it->display_texture.height() };
	}
}
