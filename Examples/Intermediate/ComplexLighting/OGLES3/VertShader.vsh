#version 300 es

/****************************************************************************
* Vertex Shader
* This code is for educational purposes, not optimized for performance.
* For best performance, use different shaders for different light setups 
* and calculate lighting in model space.
*
* The use of several complex lights might significantly affect performance 
* on some mobile platforms.
*
* For fast transformation and lighting code, please see FastTnL example.
*****************************************************************************/
#define VERTEX_ARRAY	0
#define NORMAL_ARRAY	1
#define TEXCOORD_ARRAY	2

layout (location = VERTEX_ARRAY) in highp vec3	inVertex;
layout (location = NORMAL_ARRAY) in highp vec3	inNormal;
layout (location = TEXCOORD_ARRAY) in highp vec2	inTexCoord;

uniform highp mat4  MVPMatrix;
uniform highp mat4  ModelView;
uniform highp mat3  ModelViewIT;
uniform lowp  int   iLightSel;
uniform highp vec3  LightPosition;
uniform highp vec3  LightDirection;
uniform lowp  vec3  LightColor;

out mediump vec2  TexCoord;
out lowp    vec3  DiffuseLight;
out lowp    vec3  SpecularLight;

const highp float  cShininess = 16.0;
const highp float  cSpotCutoff = 0.9; 
const highp float  cSpotExp = 40.0;

// General Blinn-Phong lighting function
//
// Calculated light is added to the varyings DiffuseLight and SpecularLight
//
void Lighting(bool bSpecular, highp vec3 normal, highp vec3 eyeDir, highp vec3 lightDir, lowp vec3 lightColor)
{
	lowp float NdotL = max(dot(normal, lightDir), 0.0);
	DiffuseLight += NdotL * lightColor;
	
	if (bSpecular && NdotL > 0.0)
	{
		highp vec3 halfVector = normalize(lightDir + eyeDir);
		highp float NdotH = max(dot(normal, halfVector), 0.0);		
		highp float specular = pow(NdotH, cShininess);
		SpecularLight += specular * lightColor;
	}	
}

void DirectionalLight(bool bSpecular, highp vec3 normal, highp vec3 vertexPos)
{
	// eye direction is the normalized inverse of the vertex position in eye space
	highp vec3 eyeDir = -normalize(vertexPos);
	
	Lighting(bSpecular, normal, eyeDir, LightDirection, LightColor);
}

void PointLight(bool bSpecular, highp vec3 normal, highp vec3 vertexPos)
{
	// calculate normalized light direction
	highp vec3 lightDir = -normalize(vertexPos - LightPosition);
	
	// eye direction is the normalized inverse of the vertex position in eye space
	highp vec3 eyeDir = -normalize(vertexPos);
	
	Lighting(bSpecular, normal, eyeDir, lightDir, LightColor);
}

void SpotLight(bool bSpecular, highp vec3 normal, highp vec3 vertexPos)
{
	// calculate normalized light direction
	highp vec3 lightDir = -normalize(vertexPos - LightPosition);
	
	// eye direction is the normalized inverse of the vertex position in eye space
	highp vec3 eyeDir = -normalize(vertexPos);
	
	// LightDirection is spot direction here
	highp float spotDot = dot(lightDir, LightDirection);
	highp float attenuation = 0.0;
	if (spotDot > cSpotCutoff)
	{
		attenuation = pow(spotDot, cSpotExp);
	}
	
	Lighting(bSpecular, normal, eyeDir, lightDir, attenuation * LightColor);
}

void main()
{
	// transform normal to eye space
	highp vec3 normal = normalize(ModelViewIT * inNormal);
	
	// transform vertex position to eye space
	highp vec3 ecPosition = vec3(ModelView * vec4(inVertex, 1.0));
	
	// initalize light intensity varyings
	DiffuseLight = vec3(0.0);
	SpecularLight = vec3(0.0);
	
	// select the light function
	if (iLightSel == 0)			DirectionalLight(false, normal, ecPosition);
	else if (iLightSel == 1)	DirectionalLight(true, normal, ecPosition);
	else if (iLightSel == 2)	PointLight(false, normal, ecPosition);
	else if (iLightSel == 3)	PointLight(true, normal, ecPosition);
	else if (iLightSel == 4)	SpotLight(false, normal, ecPosition);
	else 						SpotLight(true, normal, ecPosition);
	
	// Transform position
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);
	
	// Pass through texcoords
	TexCoord = inTexCoord;
}