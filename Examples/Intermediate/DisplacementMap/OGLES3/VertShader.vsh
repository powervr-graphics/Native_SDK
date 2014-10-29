#version 300 es

#define VERTEX_ARRAY	0
#define NORMAL_ARRAY	1
#define TEXCOORD_ARRAY	2

layout (location = VERTEX_ARRAY) in highp vec3	inVertex;
layout (location = NORMAL_ARRAY) in mediump vec3	inNormal;
layout (location = TEXCOORD_ARRAY) in mediump vec2	inTexCoord;

uniform highp   mat4  MVPMatrix;
uniform mediump vec3  LightDirection;
uniform mediump	float  DisplacementFactor;

out lowp    float  LightIntensity;
out mediump vec2   TexCoord;

uniform sampler2D  sDisMap;

void main()
{
	/* 
		Calculate the displacemnt value by taking the colour value from our texture
		and scale it by out displacement factor.
	*/
	mediump float disp = texture(sDisMap, inTexCoord).r * DisplacementFactor;

	/* 
		Transform position by the model-view-projection matrix but first
		move the untransformed position along the normal by our displacement
		value.
	*/
	gl_Position = MVPMatrix * vec4(inVertex + (inNormal * disp), 1.0);

	// Pass through texcoords
	TexCoord = inTexCoord;
	
	// Simple diffuse lighting in model space
	LightIntensity = dot(inNormal, -LightDirection);
}