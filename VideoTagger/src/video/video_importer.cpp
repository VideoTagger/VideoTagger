#include "pch.hpp"
#include "video_importer.hpp"

namespace vt
{
	video_importer::video_importer(std::string importer_id, std::string importer_display_name)
		: importer_id_{ std::move(importer_id) }, importer_display_name_{ std::move(importer_display_name) }
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
}
