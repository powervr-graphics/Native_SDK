#version 300 es

#define VERTEX_ARRAY	0
#define NORMAL_ARRAY	1
#define TEXCOORD_ARRAY	2

layout (location = VERTEX_ARRAY) in highp vec3	inVertex;
layout (location = NORMAL_ARRAY) in mediump vec3	inNormal;
layout (location = TEXCOORD_ARRAY) in mediump vec2	inTexCoord;

uniform highp   mat4  MVPMatrix;
uniform highp   mat3  Model;

// Precalculated constants used for lighting
uniform mediump   vec3  LightDir1;
uniform mediump   vec3  LightDir2;
uniform mediump   vec3  LightDir3;
uniform mediump   vec3  LightDir4;
uniform mediump   vec4  Ambient;

// varyings
out lowp    vec4  LightColour;
out mediump vec2  TexCoord;

void main()
{
	highp vec4 r1;
	highp vec3 norm, r2, r3;
	
	// Transform position
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);

	// Transform the Normal
	norm = normalize(Model * inNormal);

	// compute lighting
	r1.x =	max(0.0, dot(norm, LightDir1));	// White Light
	r1.y =	max(0.0, dot(norm, LightDir2));	// Blue Light
	r1.z =	max(0.0, dot(norm, LightDir3));	// Green Light
	r1.w =	max(0.0, dot(norm, LightDir4));	// Red Light

	LightColour.r = (r1.x + r1.w) + Ambient.r; // White Light (BGRA)
	LightColour.g = (r1.x + r1.z) + Ambient.g; // Red Light (BGRA)
	LightColour.b = (r1.x + r1.y) + Ambient.b; // Green Light (BGRA)
	LightColour.a = r1.x + Ambient.a; // Blue Light (BGRA)

	// Pass through texcoords
	TexCoord = inTexCoord;
}