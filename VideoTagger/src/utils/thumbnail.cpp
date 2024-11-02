#include "pch.hpp"
#include "thumbnail.hpp"

#include <core/app_context.hpp>

namespace vt::utils
{
	GLuint thumbnail::font_texture()
	{
		if (ctx_.fonts.count("thumbnail") == 0) return 0;
		auto font = ctx_.fonts.at("thumbnail");
		return reinterpret_cast<GLuint>(font->ContainerAtlas->TexID);
	}

	const thumbnail::font_glyph thumbnail::find_glyph(ImWchar c)
	{
		if (ctx_.fonts.count("thumbnail") == 0) return {};
		auto font = ctx_.fonts.at("thumbnail");
		font_glyph result;
		auto glyph = font->FindGlyph(c);
		if (glyph != nullptr)
		{
			result.uv0 = ImVec2{ glyph->U0, glyph->V0 };
			result.uv1 = ImVec2{ glyph->U1, glyph->V1 };
		}
		return result;
	}
}
