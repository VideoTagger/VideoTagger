#version 130

uniform sampler2D Texture;
in vec2 Frag_UV;
in vec4 Frag_Color;
out vec4 Out_Color;

void main()
{
    //float mask_value = texture(Texture, Frag_UV).r;
    
    // Multiplies the mask's '1.0' (White) against the Fill Color Alpha
    //Out_Color = vec4(Frag_Color.rgb, Frag_Color.a * mask_value);
	//Out_Color = Frag_Color * texture(Texture, Frag_UV.st);
	
	//vec4 c = Frag_Color * texture(Texture, Frag_UV.st);
    //Out_Color = vec4(1.0 - c.rgb, c.a);
	//Out_Color = vec4(1.0, 0.0, 0.0, 1.0);
	//Out_Color = Frag_Color * texture(Texture, Frag_UV.st);

	vec4 tex = texture(Texture, Frag_UV.st);
	float gray = tex.r;

	vec4 out_tex = vec4(Frag_Color.rgb, gray * Frag_Color.a);
	Out_Color = out_tex;
	//Out_Color = vec4(1,0,0,1);
	//Out_Color = Frag_Color * tex;
}
