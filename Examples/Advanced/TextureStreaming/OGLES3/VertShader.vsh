#version 300 es

#define VERTEX_ARRAY    0
#define TEXCOORD_ARRAY  1
#define NORMAL_ARRAY    2

layout(location = VERTEX_ARRAY)   in highp   vec3  inVertex;
layout(location = NORMAL_ARRAY)   in mediump vec3  inNormal;
layout(location = TEXCOORD_ARRAY) in mediump vec2  inTexCoord;

uniform highp   mat4    MVPMatrix;
#ifdef DIFFUSE
uniform mediump vec3    vLightPosition;
out lowp    float   fLightIntensity;
#endif

out mediump vec2    vTexCoord;

void main()
{
	// Transform position
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);

	// Pass through texcoords
	vTexCoord = inTexCoord;

#ifdef DIFFUSE
	// Simple diffuse lighting in model space
	mediump vec3 vLightDir = normalize(vLightPosition - inVertex);
	fLightIntensity = dot(normalize(inNormal), vLightDir);
#endif
}
