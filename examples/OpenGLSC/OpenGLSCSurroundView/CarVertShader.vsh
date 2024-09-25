#version 310 es
#define POSITION_ARRAY	0
#define NORMAL_ARRAY	1
#define TEXCOORD_ARRAY	2

layout (location = POSITION_ARRAY) in highp vec3 inVertex;
layout (location = NORMAL_ARRAY) in highp vec3 inNormal;
layout (location = TEXCOORD_ARRAY) in highp vec2 inTexCoord;

out highp vec2 vUV;
#if defined LOW_QUALITY_MATERIALS || defined HIGH_QUALITY_MATERIAL
out highp vec3 worldPosition;
out highp vec3 cameraPosition;
out highp vec3 normal;
#endif // LOW_QUALITY_MATERIALS || HIGH_QUALITY_MATERIAL

uniform highp mat4 uWorldTransform;
uniform highp mat4 uViewProjection;
uniform highp vec3 uCameraPosition;

void main()
{
	gl_Position = uViewProjection * uWorldTransform * vec4(inVertex.xyz, 1.0);
	vUV = inTexCoord;
#if defined LOW_QUALITY_MATERIALS || defined HIGH_QUALITY_MATERIAL
	worldPosition = (uWorldTransform * vec4(inVertex.xyz, 1.0)).xyz;
	cameraPosition = uCameraPosition;
	normal = normalize(transpose(inverse(mat3(uWorldTransform))) * inNormal);
#endif // LOW_QUALITY_MATERIALS || HIGH_QUALITY_MATERIAL
}
