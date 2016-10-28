#version 300 es
#define NUM_VIEWS 4
#extension GL_OVR_multiview : enable
layout(num_views = NUM_VIEWS) in;

in highp   vec3  inVertex;
in mediump vec3  inNormal;
in mediump vec2  inTexCoord;

uniform highp   mat4  MVPMatrix[NUM_VIEWS];
uniform highp   mat4  WorldViewIT[NUM_VIEWS];
uniform mediump vec3  LightDirection[NUM_VIEWS];

out mediump    float  LightIntensity;
out mediump vec2   TexCoord;


void main()
{
	// Transform position
	gl_Position = MVPMatrix[gl_ViewID_OVR] * vec4(inVertex, 1.0);
    mediump vec3 Normals = normalize(mat3(WorldViewIT[gl_ViewID_OVR]) * inNormal);
	// Pass through texcoords
	TexCoord = inTexCoord;

	// Simple diffuse lighting in model space with a touch of ambient
	LightIntensity = max(dot(Normals, -LightDirection[gl_ViewID_OVR]), 0.0);
}