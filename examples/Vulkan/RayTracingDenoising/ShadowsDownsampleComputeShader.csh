#version 450

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(set = 0, binding = 0, r16f) uniform image2D outputMip1;
layout(set = 0, binding = 1, r16f) uniform image2D outputMip2;
layout(set = 0, binding = 2, r16f) uniform image2D outputMip3;
layout(set = 0, binding = 3, r16f) uniform image2D outputMip4;
layout(set = 0, binding = 4) uniform sampler2D baseMip;

shared float g_tile[64];

void main()
{
	const ivec2 size = textureSize(baseMip, 0);
	const vec2 invSize = vec2(1.0f / size.x, 1.0f / size.y);

	uint GI = uint(gl_LocalInvocationIndex);
	uvec3 DTid = uvec3(gl_GlobalInvocationID);

	// You can tell if both x and y are divisible by a power of two with this value
	uint parity = DTid.x | DTid.y;

	// Downsample and store the 8x8 block
	vec2 centerUV = (vec2(DTid.xy) * 2.0f + 1.0f) * invSize;
	float avgPixel = textureLod(baseMip, centerUV, 0.0f).r;
	g_tile[GI] = avgPixel;

	imageStore(outputMip1, ivec2(DTid.xy), vec4(avgPixel));

	barrier();

	// Downsample and store the 4x4 block
	if ((parity & 1) == 0)
	{
		avgPixel = 0.25f * (avgPixel + g_tile[GI + 1] + g_tile[GI + 8] + g_tile[GI + 9]);
		g_tile[GI] = avgPixel;

		imageStore(outputMip2, ivec2(DTid.xy >> 1), vec4(avgPixel));
	}

	barrier();

	// Downsample and store the 2x2 block
	if ((parity & 3) == 0)
	{
		avgPixel = 0.25f * (avgPixel + g_tile[GI + 2] + g_tile[GI + 16] + g_tile[GI + 18]);
		g_tile[GI] = avgPixel;

		imageStore(outputMip3, ivec2(DTid.xy >> 2), vec4(avgPixel));
	}

	barrier();

	// Downsample and store the 1x1 block
	if ((parity & 7) == 0)
	{
		avgPixel = 0.25f * (avgPixel + g_tile[GI + 4] + g_tile[GI + 32] + g_tile[GI + 36]);

		imageStore(outputMip4, ivec2(DTid.xy >> 3), vec4(avgPixel));
	}
}
