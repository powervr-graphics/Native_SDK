#version 320 es

layout(set = 0, binding = 0) uniform highp sampler2D sTexture;
layout(location = 0) in highp vec2 vTexCoords[5];
layout(location = 0) out highp float oColor;

void main()
{
	oColor = texture(sTexture, vTexCoords[0]).r * 4.0;
	oColor += texture(sTexture, vTexCoords[1]).r;
	oColor += texture(sTexture, vTexCoords[2]).r;
	oColor += texture(sTexture, vTexCoords[3]).r;
	oColor += texture(sTexture, vTexCoords[4]).r;
	oColor *= 0.125;
}