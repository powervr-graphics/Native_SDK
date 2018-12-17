#version 320 es

layout(location = 0) in highp vec3 inVertex;
layout(location = 1) in mediump vec3 inNormal;
layout(location = 2) in mediump vec2 inTexCoord;
layout(location = 3) in mediump vec4 inTangent;

layout(std140, set = 0, binding = 0) uniform Dynamics
{
	highp mat4 view;
	highp mat4 VPMatrix;
	highp vec3 camPos;
	mediump float emissiveIntensity;
};

#define MODEL_MAT4_ARRAY_SIZE 25
layout(std140, set = 0, binding = 1) uniform Model
{
        mat4 model[MODEL_MAT4_ARRAY_SIZE];
        mat3 modelInvTranspose[MODEL_MAT4_ARRAY_SIZE];
};

layout(push_constant) uniform PushConsts {
    uint modelMtxId;
	uint materialId;
} pushConstant;

layout (location = 0) out highp vec3 outWorldPos;
layout (location = 1) out mediump vec3 outNormal;
layout (location = 2) out mediump vec2 outTexCoord;
layout(location = 3) flat out mediump int inInstanceIndex;
layout(location = 4) out mediump vec4 outTangent;
layout(location = 5) out mediump vec3 outBitTangent;

void main()
{
	outWorldPos =  (model[pushConstant.modelMtxId + uint(gl_InstanceIndex)] * vec4(inVertex, 1.0)).xyz;
	gl_Position = VPMatrix * vec4(outWorldPos, 1.0);
	outNormal  = normalize(modelInvTranspose[pushConstant.modelMtxId + uint(gl_InstanceIndex)] * inNormal);
        
	outTexCoord = inTexCoord;       
	inInstanceIndex = gl_InstanceIndex;
	outTangent = inTangent;
	outBitTangent = cross(inNormal, inTangent.xyz) * inTangent.w;    
}
