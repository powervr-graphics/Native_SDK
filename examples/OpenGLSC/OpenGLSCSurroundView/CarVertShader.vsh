#version 310 es

layout(location = 0)in highp   vec3 inVertex;
layout(location = 1)in highp   vec3 inNormal;
layout(location = 2)in mediump vec2 inTexCoord;

layout(std140, binding = 0) uniform UboDynamic
{
	highp mat4 VPMatrix;
	highp vec3 camPos;
} uboDynamic;

layout(std140, binding = 1) uniform UboPerModel
{
	highp mat4 modelMatrix;
} uboPerModel;

layout(location = 0) out highp vec3 outWorldPos;
layout(location = 1) out highp vec3 outNormal;

void main()
{
	highp vec4 worldPos = uboPerModel.modelMatrix * vec4(inVertex, 1.0);
	outNormal = normalize(transpose(inverse(mat3(uboPerModel.modelMatrix))) * inNormal);

	outWorldPos = worldPos.xyz;
	gl_Position = uboDynamic.VPMatrix * worldPos;
}
