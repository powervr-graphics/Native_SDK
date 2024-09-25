#version 320 es

layout(set = 0, binding = 0) uniform highp sampler2D sCurrentBlurredImage;
layout(set = 0, binding = 1) uniform highp sampler2D sDownsampledCurrentMipLevel;

layout(location = 0) in highp vec2 vTexCoords[9];
layout(location = 0) out highp float oColor;

const mediump float weights[9] = float[9](0.25, 0.0625, 0.125, 0.0625, 0.125, 0.0625, 0.125, 0.0625, 0.125);

layout(push_constant) uniform pushConstantsBlock{
	highp vec2 upSampleConfigs[8];
	highp float exposureBias;
};

void main()
{
	highp float sum = texture(sDownsampledCurrentMipLevel, vTexCoords[0]).r * weights[0];
	
	for(int i = 0; i < 9; ++i)
	{
		sum += texture(sCurrentBlurredImage, vTexCoords[i]).r * weights[i];
	}

	oColor = sum;
}