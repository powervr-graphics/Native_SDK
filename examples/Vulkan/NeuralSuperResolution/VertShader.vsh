#version 320 es

layout(constant_id = 0) const bool HAS_MATERIAL_TEXTURES = false;

layout(location = 0) in highp vec3 inVertex;
layout(location = 1) in highp vec3 inNormal;
layout(location = 2) in mediump vec2 inTexCoord;
layout(location = 3) in mediump vec4 inTangent;

layout(std140, set = 0, binding = 0) uniform Dynamics
{
	highp mat4 inverseViewProjectionMatrix;
    highp mat4 projectionMatrix;
    highp mat4 viewMatrixCurrentFrame;
	highp mat4 viewMatrixPreviousFrame;
	highp vec3 cameraPosition;
    highp float exposure;
    highp int screenWidth;
    highp int screenHeight;
	mediump float emissiveIntensity;
	mediump float textureLODBias;
	highp vec2 jitter;
} ubo;

layout(std140, set = 0, binding = 1) uniform Model
{
	highp mat4 modelMatrix;
};

layout(location = 0) out highp vec3 outWorldPos;
layout(location = 1) out highp vec3 outNormal;
layout(location = 2) flat out mediump int outInstanceIndex;
layout(location = 3) out highp vec4 clipPosition;
layout(location = 4) out highp vec4 clipPositionPreviousFrame;

// Material textures
layout(location = 5) out mediump vec2 outTexCoord;
layout(location = 6) out mediump vec3 outTangent;
layout(location = 7) out mediump vec3 outBiTangent;

void main()
{
	highp vec4 posTmp = modelMatrix * vec4(inVertex, 1.0);
	outInstanceIndex = gl_InstanceIndex;

	outTexCoord = inTexCoord;
	outTangent = inTangent.xyz;
	outBiTangent = cross(inNormal, inTangent.xyz) * inTangent.w;

	outNormal = normalize(transpose(inverse(mat3(modelMatrix))) * inNormal);
	outWorldPos = posTmp.xyz;

	clipPosition = ubo.projectionMatrix * ubo.viewMatrixCurrentFrame * posTmp;
	clipPositionPreviousFrame = ubo.projectionMatrix * ubo.viewMatrixPreviousFrame * posTmp;

	// Add jitter to the vertex shader output in clip space
	gl_Position = clipPosition + vec4(ubo.jitter * clipPosition.w, 0.0, 0.0);
}
