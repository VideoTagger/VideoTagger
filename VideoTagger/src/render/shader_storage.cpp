#include "shader_storage.hpp"
#include <embeds/mask_shader_frag.hpp>
#include <embeds/mask_shader_vert.hpp>
#include <embeds/mask_preview_shader_frag.hpp>

namespace vt
{
	shader_storage::shader_storage() :
		mask_shader{ vt::embed::mask_shader_vert, vt::embed::mask_shader_frag },
		mask_preview_shader{ vt::embed::mask_shader_vert, vt::embed::mask_preview_shader_frag }
	{

	}
}
