#version 310 es

layout(location = 0) in highp vec3 inVertex;
layout(location = 1) in highp vec4 inColor;
layout(location = 2) in highp vec2 inTexCoord;

uniform highp mat4 uViewProjection;
uniform highp mat4 uWorldTransform;

out highp vec3 vPosition;
out highp vec2 vTexture;
out highp vec2 vCameraWeights;

void main()
{
	highp vec3 worldVertex = vec3(uWorldTransform * vec4(inVertex, 1.0));
	gl_Position = uViewProjection * vec4(worldVertex, 1.0);
	vTexture = inTexCoord;
	vPosition = worldVertex;
	vCameraWeights = (inColor.gr / 65535.0);
}
