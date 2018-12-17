#version 310 es

const int MaxGaussianKernel = 51;
const int MaxArraySize = (MaxGaussianKernel - 1) / 2 + 1;

layout(std430, binding = 0) readonly buffer SsboBlurConfig
{
	mediump vec4 blurConfig;
	mediump float gWeights[MaxArraySize];
	mediump float gOffsets[MaxArraySize];
};

layout(binding = 0) uniform mediump sampler2D sTexture;
layout(location = 0) out mediump float oColor;

layout(location = 0) in mediump vec2 vTexCoord;

mediump float GaussianBlurFilter(mediump sampler2D tex, mediump vec2 texCoord, mediump vec2 pixelOffset)
{
	mediump float colorOut = 0.0;
	mediump float textureColor = 0.0;
	mediump vec2 texCoordOffset = vec2(0.0);
	colorOut += gWeights[0] * texture(tex, texCoord).r;

	for(int i = 1; i < int(blurConfig.z); i++)
	{
		texCoordOffset = float(i) * pixelOffset;
		textureColor = texture(tex, texCoord + texCoordOffset).r + texture(tex, texCoord - texCoordOffset).r;
		colorOut += gWeights[i] * textureColor;
	}

	return colorOut;
}

void main()
{
	oColor = GaussianBlurFilter(sTexture, vTexCoord.xy, vec2(0.0, blurConfig.y));
}
