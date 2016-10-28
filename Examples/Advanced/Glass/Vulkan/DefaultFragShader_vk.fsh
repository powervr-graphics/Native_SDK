#version 450

layout(set = 1, binding = 0) uniform sampler2D s2DMap;

layout(location = 0)in highp vec2 TexCoords;
layout(location = 1)in highp float LightIntensity;
layout(location = 0) out vec4 oColor;
void main()
{
	oColor = vec4(LightIntensity * texture(s2DMap, TexCoords).rgb, 1.0);
	// Sample texture and shade fragment
}
