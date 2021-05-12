#version 460
#extension GL_EXT_ray_tracing : require

struct VisibilityRayPayload
{
	float visibility;
};

layout(location = 0) rayPayloadInEXT VisibilityRayPayload visibilityRayPayload;

void main()
{
	// If the ray does not hit anything, we will set the visiblity to 1.0 as the light ray can reach this point.
	visibilityRayPayload.visibility = 1.0;
}