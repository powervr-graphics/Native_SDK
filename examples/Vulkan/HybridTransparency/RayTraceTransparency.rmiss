#version 460
#extension GL_EXT_ray_tracing : require

struct TransparencyRayPayload
{
	vec4 transparencyColor;         // Where to accumulate merged samples of transparent scene elements following the weighted blended order-independent transparency implementation
	float reveal;                   // Used to weight the contributions from all the transparent scene elements and the opaque scene element intersected in the final color
	bool isShadowRay;               // True means shadow ray casted from every texel in the GBuffer corresponding to an opaque scene element in the direction of the emitter. False means the ray is casted from the camera towards the direction of a transparent scene element in the GBuffer, to see through it
	int numTransparentIntersection; // Flag to count how many intersections with transparent scene elements happened
};

layout(location = 0) rayPayloadInEXT TransparencyRayPayload rayPayload;

layout(set = 0, binding = 3) uniform samplerCube skyboxImage;

void main()
{
	if(!rayPayload.isShadowRay)
	{
		// If the ray, originally from the camera due to a transparent scene element initially rasterized in the G-Buffer, did not hit any scene element, sample the skybox.
		vec3 direction   = normalize(gl_WorldRayDirectionEXT);
		vec3 reflectance = textureLod(skyboxImage, direction, 0.0).rgb;

		// Resolve the whole opaque + transparent contribution and store the result in rayPayload.transparencyColor.xyz
		rayPayload.transparencyColor.xyz = reflectance.xyz * rayPayload.reveal + rayPayload.transparencyColor.xyz * (1.0 - rayPayload.reveal); // Use this field to hold the final color
	}
}
