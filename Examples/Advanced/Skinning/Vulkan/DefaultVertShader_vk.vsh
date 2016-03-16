#version 450

layout(location = 0) in vec3 inVertex;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

// perframe / per mesh
layout(std140, set = 1, binding = 0) uniform Dynamics
{
    highp   mat4 ModelMatrix;
    highp   mat3 ModelWorldIT3x3;
};

// static
layout(std140, set = 2, binding = 0) uniform Statics
{
    mat4 VPMatrix;
    vec3 LightPos;
};

layout(location = 0) out vec2 vTexCoord;
layout(location = 1) out vec3 vWorldNormal;
layout(location = 2) out vec3 vLightDir;
layout(location = 3) out float vOneOverAttenuation;

void main()
{
	gl_Position = VPMatrix * ModelMatrix * vec4(inVertex, 1.0);

	vec3 worldPos = (ModelMatrix * vec4(inVertex, 1.0)).xyz;
	vLightDir = LightPos - worldPos;
	float light_distance = length(vLightDir);
	vLightDir /= light_distance;

	vOneOverAttenuation = 1. / (1. + .00005*light_distance*light_distance);

	vWorldNormal = ModelWorldIT3x3 * inNormal;
	// Pass through texcoords
	vTexCoord = inTexCoord;
}
 
