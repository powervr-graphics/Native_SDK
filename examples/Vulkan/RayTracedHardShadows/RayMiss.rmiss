#version 460
#extension GL_EXT_ray_tracing : require

struct VisibilityRayPayload
{
	float visibility;
};

struct hitPayload
{
	vec3 hitValue;
};

layout(location = 0) rayPayloadInEXT hitPayload payload;

void main()
{
	payload.hitValue = vec3(0.02, 0.02, 0.02);
}
