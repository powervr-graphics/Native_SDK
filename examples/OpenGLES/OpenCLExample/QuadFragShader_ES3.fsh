#version 300 es
uniform mediump sampler2D  sTexture;
in mediump vec2 TexCoord;

layout (location = 0) out lowp vec4 oColor;

void main()
{
	
	oColor = texture(sTexture, TexCoord);
}
