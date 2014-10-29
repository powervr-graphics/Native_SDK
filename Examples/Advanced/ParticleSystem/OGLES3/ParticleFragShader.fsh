#version 300 es

uniform sampler2D sTexture;

in mediump float fTexCoord;
layout (location = 0) out lowp vec4 oColour;

void main()
{
	oColour = vec4(texture(sTexture, vec2(fTexCoord,.5)).rgb, 1);
}
