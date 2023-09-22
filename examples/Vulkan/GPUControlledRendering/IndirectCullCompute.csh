#version 450
precision highp float;
layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

struct GPUSSBOMeshData
{
	mat4 model;
	vec4 spherebounds;
	vec3 pos;
	float scale;
};
// all object matrices
layout(set = 0, binding = 0) readonly buffer ObjectBuffer { GPUSSBOMeshData objects[]; }
objectBuffer;

struct GPUIndirectDrawCommandObject
{
	uint indexCount;
	uint instanceCount;
	uint firstIndex;
	int vertexOffset;
	uint firstInstance;
};
layout(set = 0, binding = 1) buffer InstanceBuffer { GPUIndirectDrawCommandObject Draws[]; }
drawBuffer;

struct GPUPerInstanceInput
{
	uint objectID;
	uint batchID;
};

layout(set = 0, binding = 2) readonly buffer GpuInputInstanceBuffer { GPUPerInstanceInput Instances[]; }
inputInstanceBuffer;

layout(set = 0, binding = 3) buffer GpuOuputVisibilityBuffer { uint IDs[]; }
visibilityInstanceBuffer;

layout(set = 0, binding = 4) uniform DrawCullData
{
	vec4 frustrumPlanes[6];
	uint CullingEnabled;
	uint DrawCount;
	float zNear;
}
cullData;

bool IsVisibleAfterFrustrumCull(uint objectIndex)
{
	uint index = objectIndex;

	vec4 sphereBounds = objectBuffer.objects[index].spherebounds;
	mat4 modelMat = objectBuffer.objects[index].model;
	float scale = objectBuffer.objects[index].scale;
	vec4 center = modelMat * vec4(sphereBounds.xyz, 1.0f); //*scale + pos;
	float radius = sphereBounds.w * scale;

	// Frustrum culling
	bool insideFrustrum = true;
	for (int i = 0; i < 6; ++i) { insideFrustrum = insideFrustrum && dot(cullData.frustrumPlanes[i], vec4(center.xyz, 1.0)) > -radius; }

	return insideFrustrum;
}

void main()
{
	uint gID = gl_GlobalInvocationID.x;

	if (gID < cullData.DrawCount)
	{
		uint objectID = inputInstanceBuffer.Instances[gID].objectID;
		bool visible = true;
		if (cullData.CullingEnabled == 1) { visible = IsVisibleAfterFrustrumCull(objectID); }
		if (visible)
		{
			uint batchIndex = inputInstanceBuffer.Instances[gID].batchID;
			uint countIndex = atomicAdd(drawBuffer.Draws[batchIndex].instanceCount, 1);
			uint instanceIndex = drawBuffer.Draws[batchIndex].firstInstance + countIndex;
			visibilityInstanceBuffer.IDs[instanceIndex] = objectID;
		}
	}
}
