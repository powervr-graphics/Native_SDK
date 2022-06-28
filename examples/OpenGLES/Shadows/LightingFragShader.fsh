#version 310 es

in mediump vec2 vTexCoord;
in highp vec3 vWorldNormal;
in highp vec3 vWorldPos;
in highp vec4 vPosition;

out mediump vec4 oColor;

layout(std140, binding = 0) uniform GlobalUBO
{
	highp mat4 ViewProjMat;
	highp mat4 ProjMat;
	highp mat4 ViewMat;
	highp mat4 ShadowMat;
	highp vec4 LightDir;
	highp vec4 LightPosVS;
	highp vec4 LightDirVS;
};

void main()
{
	mediump float ambient = 0.05;
	mediump vec3 diffuse = vec3(0.75);
	mediump vec3 color = diffuse * clamp(dot(normalize(vWorldNormal), normalize(-LightDir.xyz)), 0.0, 1.0) + diffuse * ambient;

	oColor = vec4(color, 1.0);
}