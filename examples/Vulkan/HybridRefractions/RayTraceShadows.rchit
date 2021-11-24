#version 460
#extension GL_EXT_ray_tracing : require

layout(location = 2) rayPayloadInEXT bool visibilityRayPayload;

void main()
{
	// If the ray hits something, we will set the visiblity to false as direct light rays cannot reach this point.
	visibilityRayPayload = false;
}
