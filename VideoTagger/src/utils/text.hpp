#pragma once
#include <imgui.h>
#include <string>

namespace vt::utils
{
	inline void render_text_wrapped_ellipsis(ImDrawList* draw_list, const std::string& text, const ImRect& draw_rect)
	{
		if (!draw_list or text.empty()) return;

		ImFont* font = ImGui::GetFont();
		const float font_size = ImGui::GetFontSize();
		const float max_width = draw_rect.GetWidth();
		const float max_height = draw_rect.GetHeight();

		const char* str = text.c_str();
		const char* str_end = str + text.size();

		auto text_color = ImGui::GetColorU32(ImGuiCol_Text);

		ImVec2 pos = draw_rect.Min;
		float line_height = font->FontSize;
		float y = pos.y;

		const std::string ellipsis = "...";
		const float ellipsis_width = font->CalcTextSizeA(font_size, FLT_MAX, -1.0f, ellipsis.c_str()).x;

		while (str < str_end)
		{
			// stop if no vertical space for another line
			if (y + line_height > draw_rect.Max.y)
			{
				// draw ellipsis on previous line
				float ellipsis_x = draw_rect.Max.x - ellipsis_width;
				draw_list->AddText(ImVec2(ellipsis_x, y - line_height), text_color, ellipsis.c_str());
				break;
			}

			const char* line_start = str;
			const char* line_end = str;
			float line_width = 0.0f;

			// greedy wrapping
			while (line_end < str_end)
			{
				unsigned int c;
				const char* prev = line_end;
				line_end += ImTextCharFromUtf8(&c, line_end, str_end);

				if (c == '\n') break;

				float char_width = font->CalcTextSizeA(font_size, FLT_MAX, -1.0f, prev, line_end).x;

				if (line_width + char_width > max_width)
				{
					// try ellipsis if this is last vertical space
					if (y + line_height * 2 > draw_rect.Max.y)
					{
						float allowed = max_width - ellipsis_width;
						const char* cut = line_start;
						float w = 0.0f;

						while (cut < line_end)
						{
							unsigned int c2;
							const char* prev2 = cut;
							cut += ImTextCharFromUtf8(&c2, cut, line_end);

							float cw = font->CalcTextSizeA(font_size, FLT_MAX, -1.0f, prev2, cut).x;

							if (w + cw > allowed) break;
							w += cw;
						}

						std::string final_line(line_start, cut);
						final_line += ellipsis;

						draw_list->AddText(ImVec2(pos.x, y), text_color, final_line.c_str());
						return;
					}
					break;
				}

				line_width += char_width;
			}

			// draw line
			draw_list->AddText(ImVec2(pos.x, y), text_color, line_start, line_end);

			// advance
			if (line_end < str_end and *line_end == '\n')
			{
				line_end++;
			}

			str = line_end;
			y += line_height;
		}
	}
}
