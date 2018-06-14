#version 450 core

layout (set = 2, binding = 0) uniform mediump sampler2D sTexture;

layout(location = 0) out lowp vec4 oColour;

layout(location = 0) in lowp vec4 fragColour;
layout(location = 1) in mediump vec2 texCoordOut;

void main(void)
{
	lowp vec4 texColour = texture(sTexture, texCoordOut);
	oColour = vec4(fragColour.rgb * texColour.r,texColour.a);
}