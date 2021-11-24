#version 320 es

// Output texture from the Ambient Occlusion renderpass
layout(set = 0, binding = 0) uniform mediump sampler2D AOFactor;

layout(location = 0) in highp vec2 vTexCoord;
layout(location = 0) out highp vec4 outColor;

void main()
{
	// Fragment offsets and the Gaussian blur weights
	const highp float weights[3] = float[3](0.27901, 0.44198, 0.27901);
	const highp float texOffset[3] = float[3](-1.0, 0.0, 1.0);

	// Get the current fragment size in the horizontal direction
	highp float fragSize = 1.0 / float(textureSize(AOFactor, 0).x);

	// Calculate the horizontal Gaussian blur for the current fragment
	highp float gaussianBlur = 0.0;
	for (int i = 0; i < 3; ++i) { gaussianBlur += weights[i] * texture(AOFactor, vTexCoord + vec2(fragSize * texOffset[i], 0.0)).r; }

	outColor = vec4(gaussianBlur);
}
