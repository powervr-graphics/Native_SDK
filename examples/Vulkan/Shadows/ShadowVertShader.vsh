#version 320 es

precision highp float;

layout(location = 0) in vec3 inVertex;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(std140, set = 0, binding = 0) uniform GlobalUBO
{
	mat4 ViewProjMat;
	mat4 ProjMat;
	mat4 ViewMat;
	mat4 ShadowMat;
	vec4 LightDir;
	vec4 LightPosVS;
	vec4 LightDirVS;
};

layout(push_constant) uniform PushConsts { mat4 ModelMat; };

void main() { gl_Position = ShadowMat * ModelMat * vec4(inVertex, 1.0); }
