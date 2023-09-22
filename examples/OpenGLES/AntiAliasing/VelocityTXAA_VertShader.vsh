#version 310 es
#define VERTEX_ARRAY 0
#define NORMAL_ARRAY 1
#define TEXCOORD_ARRAY 2
#define TANGENT_ARRAY 3

layout(location = VERTEX_ARRAY) in highp vec4 inVertex;
layout(location = NORMAL_ARRAY) in highp vec3 inNormal;
layout(location = TEXCOORD_ARRAY) in highp vec2 inTexCoord;
layout(location = TANGENT_ARRAY) in highp vec3 inTangent;

uniform mat4 PreModel;
uniform mat4 PreProjView;
uniform mat4 PreWorld;

uniform mat4 CurrModel;
uniform mat4 CurrProjView;
uniform mat4 CurrWorld;

uniform mediump vec3 CurrLightDir;
uniform mediump vec2 uJitter;

out mediump vec3 LightVec;
out mediump vec2 TexCoord;

out highp vec4 currentFramePosition;
out highp vec4 prevFramePosition;

void main()
{
	// current frame information for position calculation
	mat4 currFrameModelMtx = CurrModel * CurrWorld;
	mediump vec4 currFrameWorldPos = currFrameModelMtx * inVertex;
	mediump vec4 currFrameClipPos = CurrProjView * currFrameWorldPos;
	currentFramePosition = currFrameClipPos;

	// out position will be jittered position of vertex
	mediump vec4 currFrameClipPosJittered = currFrameClipPos + vec4(uJitter * currFrameClipPos.w, 0.0f, 0.0f);
	gl_Position = currFrameClipPosJittered;

	// previous frame information for velocity calculation
	mat4 prevFrameModelMtx = PreModel * PreWorld;
	mediump vec4 prevFrameWorldPos = prevFrameModelMtx * inVertex;
	mediump vec4 prevFrameClipPos = PreProjView * prevFrameWorldPos;
	prevFramePosition = prevFrameClipPos;

	// Lighting calculations
	mediump vec3 lightDirection = normalize(CurrLightDir);

	// transform light direction from model space to tangent space
	mediump vec3 bitangent = cross(inNormal, inTangent);
	mat3 tangentSpaceXform = mat3(inTangent, bitangent, inNormal);
	LightVec = lightDirection * tangentSpaceXform;

	// Calculate texture coordinates
	TexCoord = inTexCoord;
}
