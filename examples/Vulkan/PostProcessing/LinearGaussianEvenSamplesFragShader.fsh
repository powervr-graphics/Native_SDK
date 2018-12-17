#version 320 es

const int MaxGaussianKernel = 51;
const int MaxArraySize = (MaxGaussianKernel - 1) / 2 + 1;

layout(std430, set = 0, binding = 1) readonly buffer SsboBlurConfig
{
	mediump vec4 blurConfig;
	mediump float gWeights[MaxArraySize];
	mediump float gOffsets[MaxArraySize];
};

layout(set = 0, binding = 0) uniform mediump sampler2D sTexture;
layout(location = 0) out mediump float oColor;

layout(location = 0) in mediump vec2 vTexCoords[MaxArraySize];

void main()
{
	mediump float colorOut = 0.0;

	int halfKernelIndex = int(blurConfig.z) - 1;

	for(int i = 0; i < int(blurConfig.z); i++)
	{
		colorOut += gWeights[i] * (texture(sTexture, vTexCoords[halfKernelIndex - i]).r + texture(sTexture, vTexCoords[1 + i + halfKernelIndex]).r);
	}
	oColor = colorOut;
}