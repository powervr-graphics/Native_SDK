#version 460
#define VERTEX_ARRAY 0
#define NORMAL_ARRAY 1
#define TEXCOORD_ARRAY 2

// vertex attributes
layout(location = VERTEX_ARRAY) in vec4 inVertex;
layout(location = NORMAL_ARRAY) in vec3 inNormal;
layout(location = TEXCOORD_ARRAY) in vec2 inTexCoord;

layout(set = 0, binding = 0) uniform PerMesh
{
	highp mat4 proj; // projection matrix
};

// SSBO ObjectData
struct GPUSSBOMeshData
{
	highp mat4 modelMatrix;
	highp vec4 origin_rad; // bounds
	vec3 pos;
	float scale;
};

// SSBO per instance buffer
layout(set = 1, binding = 1) readonly buffer ObjectBuffer { GPUSSBOMeshData objects[]; }
objectBuffer;

// SSBO visibility buffer
layout(set = 1, binding = 0) readonly buffer InstanceBuffer { uint IDs[]; }
instanceBuffer;

layout(location = 0) out mediump vec2 TexCoord;
layout(location = 1) flat out mediump uint outDrawID;
layout(location = 2) out mediump vec3 Normal;
layout(location = 3) out mediump vec3 FragPos;

void main()
{
	uint index = instanceBuffer.IDs[gl_InstanceIndex];
	mat4 modelMat = objectBuffer.objects[index].modelMatrix;

	vec4 worldPos = modelMat * vec4(inVertex.xyz, 1.0);
	gl_Position = proj * worldPos;

	TexCoord = inTexCoord;
	outDrawID = gl_DrawID; // for indexing into our array of textures
	Normal = normalize(transpose(inverse(mat3(modelMat))) * inNormal);
	FragPos = vec3(modelMat * vec4(inVertex.xyz, 1.0));
}
