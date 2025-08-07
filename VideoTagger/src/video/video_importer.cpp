#include "pch.hpp"
#include "video_importer.hpp"
#include <utils/random.hpp>

namespace vt
{
	video_id_t video_importer::generate_video_id()
	{
		return utils::random::get_uuid();
	}
}
