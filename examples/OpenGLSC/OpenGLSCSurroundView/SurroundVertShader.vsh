#version 310 es

layout (location = 0) in  highp vec3  inVertex;
layout (location = 1) in  highp vec4  inColor;
layout (location = 2) in  highp vec2  inTexCoord;

layout(std140, binding = 0) uniform UboDynamic
{
	highp mat4 VPMatrix;
	highp vec3 camPos;
} uboDynamic;

layout(std140, binding = 1) uniform UboPerModel
{
	highp mat4 modelMatrix;
} uboPerModel;

uniform highp vec2 uBlendZone;
uniform highp mat4 uRotationMat;

out highp vec3 outPosition;
out highp vec2 outTexture;
out highp vec2 outCameraWeights;

void main()
{
	mediump float blendBegin = uBlendZone.x;
	mediump float blendEnd = uBlendZone.y;
	mediump float blend = clamp((inTexCoord.x-blendBegin)/(blendEnd-blendBegin), 0.0, 1.0);

	highp vec4 worldPos = uRotationMat * uboPerModel.modelMatrix * vec4(inVertex, 1.0);
	outTexture = inTexCoord;
	outPosition = worldPos.xyz;
	outCameraWeights = vec2(blend, 1.0 - blend);
	gl_Position = uboDynamic.VPMatrix * worldPos;
}
