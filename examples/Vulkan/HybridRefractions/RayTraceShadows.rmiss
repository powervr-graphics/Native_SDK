#version 460
#extension GL_EXT_ray_tracing : require

layout(location = 2) rayPayloadInEXT bool visibilityRayPayload;

void main()
{
	// If the ray does not hit anything, we will set the visiblity to true as the light ray can reach this point.
	visibilityRayPayload = true;
}
