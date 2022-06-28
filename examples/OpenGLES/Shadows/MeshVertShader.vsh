#version 310 es

precision highp float;

in vec3 inVertex;
in vec3 inNormal;
in vec2 inTexCoord;

layout(std140, binding = 0) uniform GlobalUBO
{
	mat4 ViewProjMat;
	mat4 ProjMat;
	mat4 ViewMat;
	mat4 ShadowMat;
	vec4 LightDir;
	vec4 LightPosVS;
	vec4 LightDirVS;
};

uniform mat4 ModelMat;

out vec2 vTexCoord;
out vec3 vWorldNormal;
out vec3 vWorldPos;
out vec4 vPosition;

void main()
{
	vec4 worldPos = ModelMat * vec4(inVertex, 1.0);
	vWorldPos = worldPos.xyz;

	gl_Position = ViewProjMat * worldPos;
	vPosition = gl_Position;

	vWorldNormal = mat3(ModelMat) * inNormal;

	// Pass through texcoords
	vTexCoord = inTexCoord;
}
