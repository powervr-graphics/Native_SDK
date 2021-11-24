#version 320 es

// Output texture from the horizontally blurred ambient occlusion renderpass
layout(set = 0, binding = 0) uniform mediump sampler2D partiallyBlurred;

layout(location = 0) in highp vec2 vTexCoord;
layout(location = 0) out highp vec4 outColor;

void main()
{
	// Fragment offsets and the Gaussian blur weights
	const highp float weights[3] = float[3](0.27901, 0.44198, 0.27901);
	const highp float texOffset[3] = float[3](-1.0, 0.0, 1.0);

	// Get the current fragment size in the vertical direction
	highp float fragSize = 1.0 / float(textureSize(partiallyBlurred, 0).y);

	// Calculate the vertical Gaussian blur for the current fragment
	highp float gaussianBlur = 0.0;
	for (int i = 0; i < 3; ++i) { gaussianBlur += weights[i] * texture(partiallyBlurred, vTexCoord + vec2(0.0, fragSize * texOffset[i])).r; }

	outColor = vec4(gaussianBlur);
}
