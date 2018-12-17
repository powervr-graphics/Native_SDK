#version 310 es

const int MaxGaussianKernel = 51;
const int MaxArraySize = (MaxGaussianKernel - 1) / 2 + 1;

layout(std430, binding = 1) readonly buffer SsboBlurConfig
{
	mediump vec4 blurConfig;
	mediump float gWeights[MaxArraySize];
	mediump float gOffsets[MaxArraySize];
};

layout(location = 0) out mediump vec2 vTexCoords[MaxArraySize];

void main()
{
	mediump vec2 texcoord = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
	gl_Position = vec4(texcoord * 2.0 + -1.0, 0.0, 1.0);

	int halfKernelIndex = int(blurConfig.z) - 1;

	vTexCoords[0] = texcoord;

	for (int i = 1; i < int(blurConfig.z); i++)
	{
		vTexCoords[halfKernelIndex - i + 1] = texcoord - gOffsets[i] * vec2(blurConfig.x, 0.0);
		vTexCoords[i + halfKernelIndex] = texcoord + gOffsets[i] * vec2(blurConfig.x, 0.0);
	}
}