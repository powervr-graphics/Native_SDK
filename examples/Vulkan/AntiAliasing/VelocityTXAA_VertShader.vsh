#version 320 es

#define VERTEX_ARRAY 0
#define NORMAL_ARRAY 1
#define TEXCOORD_ARRAY 2
#define TANGENT_ARRAY 3

layout(location = VERTEX_ARRAY) in highp vec4 inVertex;
layout(location = NORMAL_ARRAY) in highp vec3 inNormal;
layout(location = TEXCOORD_ARRAY) in highp vec2 inTexCoord;
layout(location = TANGENT_ARRAY) in highp vec3 inTangent;

layout (set = 1, binding = 0) uniform UboTAA
{
    highp mat4 PreModel;
    highp mat4 PreProjView;
    highp mat4 PreWorld;

    highp mat4 CurrModel;
    highp mat4 CurrProjView;
    highp mat4 CurrWorld;

    highp vec3 CurrLightDir;
    highp vec2 uJitter;
} ubo;

layout(location = 0) out mediump vec3 LightVec;
layout(location = 1) out mediump vec2 TexCoord;

layout(location = 2) out highp vec4 currentFramePosition;
layout(location = 3) out highp vec4 prevFramePosition;

void main()
{
	// current frame information for position calculation
    mat4 currFrameModelMtx = ubo.CurrModel * ubo.CurrWorld;
    mediump vec4 currFrameWorldPos = currFrameModelMtx * inVertex;
    mediump vec4 currFrameClipPos = ubo.CurrProjView * currFrameWorldPos;
    currentFramePosition = currFrameClipPos;

	// out position will be jittered position of vertex
    mediump vec4 currFrameClipPosJittered = currFrameClipPos + vec4(ubo.uJitter * currFrameClipPos.w, 0.0f, 0.0f);
    gl_Position = currFrameClipPosJittered;

	// previous frame information for velocity calculation
    mat4 prevFrameModelMtx = ubo.PreModel * ubo.PreWorld;
    mediump vec4 prevFrameWorldPos = prevFrameModelMtx * inVertex;
    mediump vec4 prevFrameClipPos = ubo.PreProjView * prevFrameWorldPos;
    prevFramePosition = prevFrameClipPos;

	// Lighting calculations
    mediump vec3 lightDirection = normalize(ubo.CurrLightDir);

	// transform light direction from model space to tangent space
    mediump vec3 bitangent = cross(inNormal, inTangent);
    mat3 tangentSpaceXform = mat3(inTangent, bitangent, inNormal);
    LightVec = lightDirection * tangentSpaceXform;

	// Calculate texture coordinates
    TexCoord = inTexCoord;
}
