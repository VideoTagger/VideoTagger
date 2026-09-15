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

void main()
{
	vec4 tex = texture(Texture, Frag_UV.st);
	float gray = tex.r;

	Out_Color = render_default(tex, gray, Frag_Color);
}
