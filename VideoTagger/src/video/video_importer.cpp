#include "pch.hpp"
#include "video_importer.hpp"
#include <utils/uuid.hpp>

namespace vt
{
	video_id_t video_importer::generate_video_id()
	{
		return utils::uuid::get();
	}

	video_importer::video_importer(std::string importer_id, std::string importer_display_name, std::string importer_display_icon)
		: importer_id_{ std::move(importer_id) }, importer_display_name_{ std::move(importer_display_name) }, importer_display_icon_{ std::move(importer_display_icon) }
	{
	}

	const std::string& video_importer::importer_id() const
	{
		return importer_id_;
	}

	const std::string& video_importer::importer_display_name() const
	{
		return importer_display_name_;
	}

	const std::string& video_importer::importer_display_icon() const
	{
		return importer_display_icon_;
	}
}
