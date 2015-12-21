#version 300 es
uniform sampler2D  sTexture;
uniform sampler2D  sBlurTexture;

uniform mediump float sTexFactor;
uniform mediump float sBlurTexFactor;


in mediump vec2 TexCoord;

layout (location = 0) out lowp vec4 oColor;

void main()
{
	oColor = (texture(sTexture, TexCoord) * sTexFactor) + (texture(sBlurTexture, TexCoord) * sBlurTexFactor);
}
