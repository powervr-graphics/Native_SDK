#version 300 es

#define VERTEX_ARRAY	0
#define NORMAL_ARRAY	1
#define TEXCOORD_ARRAY	2

layout (location = VERTEX_ARRAY) in highp vec4	inVertex;
layout (location = NORMAL_ARRAY) in highp vec3	inNormal;
layout (location = TEXCOORD_ARRAY) in highp vec2	inTexCoord;

uniform highp   mat4  MVPMatrix;
uniform highp   mat4  ModelViewMatrix;
uniform highp vec3  LightDirection;
// fog uniforms
uniform lowp    int    iFogMode;
uniform highp float  FogDensity;
uniform highp float  FogEnd;
uniform highp float  FogRcpEndStartDiff;

out mediump vec2  TexCoord;
out lowp    vec3  DiffuseLight;
out lowp    vec3  FogIntensity;

void main()
{
	// transform position to view space as we need the distance to the eye for fog
	highp vec3 viewPos = vec3(ModelViewMatrix * inVertex);
	highp float eyeDist = length(viewPos);
	
	// transform vertex position
	gl_Position = MVPMatrix * inVertex;
	
	// texcoords pass through
	TexCoord = inTexCoord;

	// calculate lighting
	// We use a directional light with direction given in model space
	lowp float DiffuseIntensity = dot(inNormal, normalize(LightDirection));
	
	// clamp negative values and add some ambient light
	DiffuseLight = vec3(max(DiffuseIntensity, 0.0) * 0.5 + 0.5);

	
	// select fog function. 1 is linear, 2 is exponential, 3 is exponential squared, 0 is no fog.
	highp float fogIntensity = 1.0;
	if(iFogMode == 1)
	{
		fogIntensity = (FogEnd - eyeDist) * FogRcpEndStartDiff;
	}
	else if(iFogMode >= 2)
	{
		highp float scaledDist = eyeDist * FogDensity;
		if (iFogMode == 3)
		{
			scaledDist *= scaledDist;
		}
		fogIntensity = exp2(-scaledDist);

	}

	// clamp the intensity within a valid range
	FogIntensity = vec3(clamp(fogIntensity, 0.0, 1.0));
}
