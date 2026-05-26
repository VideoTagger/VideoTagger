#version 130

uniform int u_mask_display_mode = 0; // 0 = default, 1 = diff
uniform float u_pattern_scale = 1.0;

uniform sampler2D Texture;
in vec2 Frag_UV;
in vec4 Frag_Color;
out vec4 Out_Color;

vec4 render_default(vec4 tex, float gray, vec4 fill)
{
	return vec4(fill.rgb, gray * fill.a);
}

vec4 render_pattern(vec2 uv, vec4 tex, float gray, vec4 fill)
{
	int col_u8 = int(floor(gray * 255.0 + 0.5));

	// Diagonal stripe pattern (bottom-left to top-right).
	float scale = ((u_pattern_scale > 0.0) ? u_pattern_scale : 1.0) * 100.0;
	float diag = (uv.x + uv.y) * scale;
	float stripe = mod(floor(diag), 2.0);
	bool is_stripe = stripe > 0.5;

	bool is_outline = false;
	vec2 tex_offset = 1.0 / vec2(textureSize(Texture, 0));

	for(int x = -1; x <= 1; x++)
	{
		for(int y = -1; y <= 1; y++)
		{
			if(x == 0 && y == 0) continue;
			float n_gray = texture(Texture, uv + vec2(x, y) * tex_offset).r;
			int n_col_u8 = int(floor(n_gray * 255.0 + 0.5));
			if(n_col_u8 != col_u8)
			{
				is_outline = true;
			}
		}
	}

	vec4 base = render_default(tex, gray, fill);
	vec4 fg_line = vec4(0.0, 1.0, 0.0, fill.a);
	vec4 bg_line = vec4(1.0, 0.0, 0.0, fill.a);

	bool highlight = is_outline || is_stripe;

	if (col_u8 == 1) // GC_FGD
	{
		return highlight ? Frag_Color : base;
	}
	if (col_u8 == 2) // GC_PR_BGD
	{
		return highlight ? bg_line : base;
	}
	if (col_u8 == 3) // GC_PR_FGD
	{
		return highlight ? fg_line : base;
	}
	return base;
}

void main()
{
	vec4 tex = texture(Texture, Frag_UV.st);
	float gray = tex.r;

	if (u_mask_display_mode == 1)
	{
		Out_Color = render_pattern(Frag_UV.st, tex, gray, Frag_Color);
	}
	else
	{
		Out_Color = render_default(tex, gray, Frag_Color);
	}
}
