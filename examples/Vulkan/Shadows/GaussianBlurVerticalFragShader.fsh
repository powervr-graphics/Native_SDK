#version 320 es

precision highp float;

layout(set = 0, binding = 0) uniform sampler2D sIntermediateMap;

layout (location = 0) in mediump vec2 vTexCoord;

layout(location = 0) out vec4 oColor;

layout(push_constant) uniform GAUSSIAN1D 
{ 
	vec4 factors[4];
	layout(offset = 64) uvec2 blurSizeShadowMapSize;
} _gaussian;

void main()
{
	vec4 value = vec4(0.0);
	vec2 texelIncrement = (vec2(1.0f) / vec2(float(_gaussian.blurSizeShadowMapSize.y))) * vec2(0.0f, 1.0f); 

	for (int y = -1 * int(_gaussian.blurSizeShadowMapSize.x); y <= int(_gaussian.blurSizeShadowMapSize.x); y++) 
	{
		vec4 samp = texture(sIntermediateMap, vTexCoord + texelIncrement * float(y));
		int gaussianIndex = y + int(_gaussian.blurSizeShadowMapSize.x);
		value += samp * _gaussian.factors[gaussianIndex / 4][gaussianIndex % 4];
	}
	oColor = value;
}