#version 300 es
uniform sampler2D  sTexture;

uniform lowp float  AlphaReference;

in mediump vec2  TexCoord;

layout (location = 0) out lowp vec4 oColour;

void main()
{
	lowp vec4 color = texture(sTexture, TexCoord);
	if (color.a < AlphaReference) 
	{
		discard;
	}
	oColour = color;
}
