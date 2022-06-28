#version 310 es

precision highp float;

precision highp float;

uniform sampler2D sIntermediateMap;

in mediump vec2 vTexCoord;

out vec4 oColor;

uniform vec4 gaussianFactors[4];
uniform uvec2 blurSizeShadowMapSize;

void main()
{
	vec4 value = vec4(0.0);
	vec2 texelIncrement = (vec2(1.0f) / vec2(float(blurSizeShadowMapSize.y))) * vec2(0.0f, 1.0f); 

	for (int y = -1 * int(blurSizeShadowMapSize.x); y <= int(blurSizeShadowMapSize.x); y++) 
	{
		vec4 samp = texture(sIntermediateMap, vTexCoord + texelIncrement * float(y));
		int gaussianIndex = y + int(blurSizeShadowMapSize.x);
		value += samp * gaussianFactors[gaussianIndex / 4][gaussianIndex % 4];
	}
	oColor = value;
	//oColor = texture(sIntermediateMap, vTexCoord);
}