#version 460
#extension GL_EXT_ray_tracing : require

struct VisibilityRayPayload
{
	float visibility;
};

layout(location = 0) rayPayloadInEXT VisibilityRayPayload visibilityRayPayload;

void main()
{
	// If the ray hits something, we will set the visiblity to 1.0 as direct light rays cannot reach this point.
	visibilityRayPayload.visibility = 0.0;
}
