#version 300 es

#define VERTEX_ARRAY	0
layout (location = VERTEX_ARRAY) in highp vec3	inVertex;

uniform highp mat4  ModelViewMatrix;
uniform highp mat4  MVPMatrix;
uniform highp vec3  EyePosition;		// Eye (aka Camera) positon in model-space
uniform mediump vec2 BumpTranslation0;
uniform mediump vec2 BumpScale0;
uniform mediump vec2 BumpTranslation1;
uniform mediump vec2 BumpScale1;
 
out mediump vec2 BumpCoord0;
out mediump vec2 BumpCoord1;
out highp   vec3 WaterToEye;
out mediump float WaterToEyeLength;

void main()
{
	// Convert each vertex into projection-space and output the value
	highp vec4 vInVertex = vec4(inVertex, 1.0);
	gl_Position = MVPMatrix * vInVertex;

	// The texture coordinate is calculated this way to reduce the number of attributes needed
	mediump vec2 vTexCoord = inVertex.xz;

	// Scale and translate texture coordinates used to sample the normal map - section 2.2 of white paper
	BumpCoord0 = vTexCoord.xy * BumpScale0;
	BumpCoord0 += BumpTranslation0;
	
	BumpCoord1 = vTexCoord.xy * BumpScale1;
	BumpCoord1 += BumpTranslation1;
	
	/* 	
		The water to eye vector is used to calculate the Fresnel term
		and to fade out perturbations based on distance from the viewer
	*/
	WaterToEye = EyePosition - inVertex;
	WaterToEyeLength = length(WaterToEye);
}
