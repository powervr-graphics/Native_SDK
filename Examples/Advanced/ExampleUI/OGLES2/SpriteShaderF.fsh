uniform sampler2D Texture;

varying highp vec2 TexCoord;
varying lowp vec4 RGBA;

void main()
{
	lowp vec4 rgba = texture2D(Texture, TexCoord) * RGBA;
#ifdef DISPLAY_SPRITE_ALPHA
	if(rgba.a < 0.1)
	{
		rgba.a = 0.6;
		rgba.rgb = vec3(1.0, 0.0, 0.0);
	}
#endif	
	gl_FragColor = rgba;
}