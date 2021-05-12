#version 460
#extension GL_EXT_ray_tracing : require

struct ReflectionRayPayload
{
	vec3 Li;
};

layout(location = 0) rayPayloadInEXT ReflectionRayPayload reflectionRayPayload;

layout(set = 3, binding = 0) uniform samplerCube skyboxImage;
layout(set = 3, binding = 1) uniform samplerCube prefilteredImage;
layout(set = 3, binding = 2) uniform samplerCube irradianceImage;
layout(set = 3, binding = 3) uniform sampler2D brdfLUT;

void main()
{
	// If the ray doesn't hit anything we will sample the skybox. 
	// Since this sample scene contains materials with zero roughness, we can simply sample from the original skybox cubemap.
	reflectionRayPayload.Li = textureLod(skyboxImage, normalize(gl_WorldRayDirectionEXT), 0.0f).rgb;
}