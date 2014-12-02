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
uniform highp   vec4  cAr;
uniform highp   vec4  cAg;
uniform highp   vec4  cAb;

uniform highp	vec4  cBr;
uniform highp	vec4  cBg;
uniform highp	vec4  cBb;

uniform highp	vec3  cC;

// varyings
out lowp    vec4  LightColour;
out mediump vec2  TexCoord;

void main()
{
	highp vec4 r0, r1;
	highp vec3 r2, r3;
	
	// Transform position
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);

	// Transform the Normal and add a homogenous 1
	r0 = vec4(Model * inNormal, 1.0);

	// Compute 1st 4 basis functions - linear + constant
	// r0 is the normal with a homegenous 1
	// c* are precomputed constants
	r2.x = dot(cAr, r0);
	r2.y = dot(cAg, r0);
	r2.z = dot(cAb, r0);
	
	// Compute polynomials for the next 4 basis functions
	r1 = r0.yzzx * r0.xyzz; // r1 is { yx, zy, z^2, xz}

	// Add contributions and store them in r3
	r3.x = dot(cBr, r1);
	r3.y = dot(cBg, r1);
	r3.z = dot(cBb, r1);

	// Compute the final basis function x^2 - y^2
	r0.z = r0.y * r0.y;
	r0.w = (r0.x * r0.x) - r0.z;
	
	// Combine the first 2 sets : 8 basis functions
	r1.xyz = r2 + r3;

	// Add in the final 9th basis function to create the final RGB Lighting
	LightColour.xyz = (cC * r0.w) + r1.xyz;
	
	// Set light alpha to 1.0
	LightColour.a = 1.0;

	// Pass through texcoords
	TexCoord = inTexCoord;
}