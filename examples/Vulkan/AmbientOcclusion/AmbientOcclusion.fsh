#version 320 es
// Hard coded AO parameters
#define SSAO_KERNEL_SIZE 32
#define SSAO_RADIUS 0.4
#define SSAO_BIAS 0.025

// Output textures from the G buffer render pass
layout(set = 0, binding = 0) uniform mediump sampler2D gBufferNormal;
layout(set = 0, binding = 1) uniform mediump sampler2D gDepthBuffer;

// Uniform buffer to set the ambient occlusion parameters
layout(set = 1, binding = 0) uniform sampleParams
{
	highp vec3[SSAO_KERNEL_SIZE] SamplePositions;
	highp vec3[3][3] SampleRotations;
	highp mat4 Projection;
	highp mat4 ProjectionInv;
};

// Screen Space coordinate passed from vertex shader
layout(location = 0) in highp vec2 vTexCoord;

// Output to the ambient occlusion attachment
layout(location = 0) out highp float outColor;

highp vec3 depthToViewSpace(highp float depth, highp vec2 screenSpaceCoord)
{
	highp vec4 clipSpace = vec4(screenSpaceCoord * 2.0 - 1.0, depth, 1.0);
	highp vec4 viewSpace = ProjectionInv * clipSpace;
	// Perform the perspective divide
	viewSpace /= viewSpace.w;
	return viewSpace.xyz;
}

void main()
{
	// Calculate the view space position for the current fragment that we are calculating AO for
	highp vec3 fragPos = depthToViewSpace(texture(gDepthBuffer, vTexCoord).r, vTexCoord);

	// Produce a matrix that will translate the samples from Tangent space to view space
	// Tangent vector is reliant on the choice of random vector, which will change the orientation
	// for the TBN matrix, which in turn causes per pixel variation
	highp vec3 normal = texture(gBufferNormal, vTexCoord).xyz;
	highp vec3 randomVec = SampleRotations[int(gl_FragCoord.x) % 3][int(gl_FragCoord.y) % 3];
	highp vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	highp vec3 bitangent = cross(normal, tangent);
	highp mat3 TBN = mat3(tangent, bitangent, normal);

	// Start calculating occlusion
	highp float occlusion = 0.0;
	for (int i = 0; i < SSAO_KERNEL_SIZE; ++i)
	{
		// Get the current sample's view space position
		highp vec3 samplePosition = TBN * SamplePositions[i];
		samplePosition = fragPos + samplePosition * SSAO_RADIUS;

		// Transfer the sample's position into screen space, use those coordinates to get
		// where in the scene to compare to the sample
		highp vec4 screenspace = vec4(samplePosition, 1.0);
		screenspace = Projection * screenspace; // from view to clip space
		screenspace.xyz /= screenspace.w; // perspective divide
		screenspace.xyz = screenspace.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0

		// get the scene depth at the sample's screen space position
		highp float sceneDepth = depthToViewSpace(texture(gDepthBuffer, screenspace.xy).r, screenspace.xy).z;

		// compare the sample's depth to the scene's depth
		if (sceneDepth >= samplePosition.z + SSAO_BIAS)
		{
			// Do a range check, this prevents scene points outside the sample radius from contributing to the AO
			highp float rangeCheck = smoothstep(0.0, 1.0, 0.1 / abs(sceneDepth - fragPos.z));
			occlusion += rangeCheck;
		}
	}
	outColor = 1.0 - occlusion / float(SSAO_KERNEL_SIZE);
}
