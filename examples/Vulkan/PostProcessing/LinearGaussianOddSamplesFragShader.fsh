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

	colorOut += gWeights[0] * texture(sTexture, vTexCoords[0]).r;

	for(int i = 1; i < int(blurConfig.z); i++)
	{
		colorOut += gWeights[i] * (texture(sTexture, vTexCoords[halfKernelIndex - i + 1]).r + texture(sTexture, vTexCoords[i + halfKernelIndex]).r);
	}
	oColor = colorOut;
}
