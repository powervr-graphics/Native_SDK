#version 320 es

layout(location = 0) in highp vec3 inVertex;
layout(location = 1) in mediump vec3 inNormal;
layout(location = 2) in mediump vec2 inTexCoord;

layout(std140, set = 0, binding = 0) uniform GlobalUBO
{
	highp mat4 ViewProjMat;
	highp mat4 ProjMat;
	highp mat4 ViewMat;
	highp mat4 ShadowMat;
	highp vec4 LightDir;
	highp vec4 LightPosVS;
	highp vec4 LightDirVS;
};

layout(push_constant) uniform PushConsts { highp mat4 ModelMat; };

layout(location = 0) out mediump vec2 vTexCoord;
layout(location = 1) out highp vec3 vWorldNormal;
layout(location = 2) out highp vec3 vWorldPos;
layout(location = 3) out highp vec4 vPosition;

void main()
{
	highp vec4 worldPos = ModelMat * vec4(inVertex, 1.0);
	vWorldPos = worldPos.xyz;

	gl_Position = ViewProjMat * worldPos;
	vPosition = gl_Position;

	vWorldNormal = mat3(ModelMat) * inNormal;

	// Pass through texcoords
	vTexCoord = inTexCoord;
}
