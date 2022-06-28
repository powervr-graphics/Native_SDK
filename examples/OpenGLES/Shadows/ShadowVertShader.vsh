#version 310 es

in highp vec3 inVertex;
in mediump vec3 inNormal;
in mediump vec2 inTexCoord;

layout(std140, binding = 0) uniform GlobalUBO
{
	highp mat4 ViewProjMat;
	highp mat4 ProjMat;
	highp mat4 ViewMat;
	highp mat4 ShadowMat;
	highp vec4 LightDir;
	highp vec4 LightPosVS;
	highp vec4 LightDirVS;
};

uniform highp mat4 ModelMat;

void main()
{
	gl_Position = ShadowMat * ModelMat * vec4(inVertex, 1.0);
}
