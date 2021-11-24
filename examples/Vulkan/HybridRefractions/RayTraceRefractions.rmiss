#version 460
#extension GL_EXT_ray_tracing : require

struct ReflectionRayPayload
{
	vec3 Li; // Incident radiance
	uint depth;
	bool inside;
	float indexOfRefraction;
};

layout(location = 0) rayPayloadInEXT ReflectionRayPayload reflectionRayPayload;

layout(set = 0, binding = 3) uniform samplerCube skyboxImage;

void main()
{
	// If the ray doesn't hit anything we will sample the skybox.
	vec3 direction = normalize(gl_WorldRayDirectionEXT);
	reflectionRayPayload.Li = textureLod(skyboxImage, direction, 0.0).rgb;
}
