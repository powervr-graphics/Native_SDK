#version 300 es
uniform sampler2D Texture;

in highp vec2 TexCoord;
in lowp vec4 RGBA;

layout (location = 0) out lowp vec4 oColour;

void main()
{
	lowp vec4 rgba = texture(Texture, TexCoord) * RGBA;
#ifdef DISPLAY_SPRITE_ALPHA
	if(rgba.a < 0.1)
	{
		rgba.a = 0.6;
		rgba.rgb = vec3(1.0, 0.0, 0.0);
	}
#endif	
	oColour = rgba;
}