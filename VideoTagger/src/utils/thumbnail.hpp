#pragma once
#include <SDL.h>
#include <SDL_opengl.h>
#include <imgui.h>

namespace vt::utils
{
	struct thumbnail
	{
		struct font_glyph
		{
			ImFontGlyph* glyph{};
			ImVec2 uv0{};
			ImVec2 uv1{};
		};

		thumbnail() = delete;

		static GLuint font_texture();
		static const font_glyph find_glyph(ImWchar c);
		
		static constexpr ImWchar video_group_icon{ 0xE04A };
		static constexpr ImWchar video_icon{ 0xE02C };
		static constexpr ImWchar download_icon{ 0xF090 };
		static constexpr ImWchar file_icon{ 0xE66D };
		static constexpr ImWchar folder_icon{ 0xE2C7 };
	};

}
