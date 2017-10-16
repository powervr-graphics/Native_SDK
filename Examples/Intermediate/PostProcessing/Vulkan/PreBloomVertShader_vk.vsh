#version 450

#define VERTEX_ARRAY	0
#define NORMAL_ARRAY	1
#define TEXCOORD_ARRAY	2

layout(location = VERTEX_ARRAY) in highp vec3 inVertex;
layout(location = NORMAL_ARRAY) in mediump vec3	inNormal;
layout(location = TEXCOORD_ARRAY) in mediump vec2 inTexCoord;

layout(location = 0) out highp float LightIntensity;
layout(location = 1) out mediump vec2 TexCoord;

layout(std140, set = 2, binding = 0) uniform PerMesh
{
	highp mat4 MVInv;
	highp mat4 MVPMatrix;
	mediump vec3 LightDirection;
};

layout(set = 3, binding = 0) uniform Static
{
	highp float Shininess;
};

void main()
{
	// Transform position
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);
	TexCoord = inTexCoord;
	LightIntensity = max(dot(inNormal, -LightDirection), 0.0) + .2/*ambient*/;
	// if the light is behind no specular reflection
	if (dot(normalize(inNormal), -LightDirection) > 0.0)
	{
		vec3 viewDir = vec3(normalize((MVInv * vec4(0, 0, 0, 1)) - vec4(inVertex, 1.0)));
		LightIntensity += Shininess * 2. * pow(max(0.0, dot(reflect(-LightDirection, inNormal), viewDir)), 32.0);

		LightIntensity += .2 * 2. * pow(max(0.0, dot(reflect(-LightDirection, inNormal), viewDir)), 32.0);
	}
}