#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_nonuniform_qualifier : enable

// Raytracing payload to send pack to the raygen shader, the color written to the ouput image
struct hitPayload
{
	vec3 hitValue;
};
layout(location = 0) rayPayloadInEXT hitPayload payload;

// Raytracing payload to check if a ray that hits geometry is in shadow or not
struct shadowPayload
{
	uint miss;
};
layout(location = 1) rayPayloadEXT shadowPayload shdPayload;

// Uniform buffer to encapsulate the light in the scene.
layout(set = 1, binding = 1) uniform LightDataUBO
{
	highp vec4 lightPosition;
	highp vec4 lightColor;
	highp float lightIntensity;
};

// The top level acceleration structure to be used to trace the shadow rays
layout(set = 2, binding = 0) uniform accelerationStructureEXT topLevelAS;

// Bindless resources, they are stored as a array indexed by the object ID, in this case each instance is a unique object
// The vertex buffers inside of the acceleration structure is currently fixed and must match pvr::utils::ASVertexFormat struct
struct Vertex
{
	vec3 pos;
	vec3 nrm;
	vec2 texCoord;
	vec3 tangent;
};
layout(set = 2, binding = 1, scalar) buffer Verticies { Vertex v[]; }
vertices[];

// Index buffers
layout(set = 2, binding = 2) buffer Indices { uint i[]; }
indices[];

// Instance transforms to world space
layout(set = 2, binding = 3) buffer Transforms { mat4 t[]; }
transforms[];

layout(set = 2, binding = 4) buffer Materials { vec4 m[]; }
materials[];

// Use an extension to be able to interpolate between the vertices
hitAttributeEXT vec2 attribs;

void main()
{
	// Get the object ID of the instance that this ray has hit, in this case, since each instance is a unique object just use the instance ID
	uint objID = gl_InstanceID;

	// indices of the triangle we hit
	ivec3 ind = ivec3(indices[nonuniformEXT(objID)].i[3 * gl_PrimitiveID + 0], //
		indices[nonuniformEXT(objID)].i[3 * gl_PrimitiveID + 1], //
		indices[nonuniformEXT(objID)].i[3 * gl_PrimitiveID + 2]); //

	// Vertices of the hit triangle
	Vertex v0 = vertices[nonuniformEXT(objID)].v[ind.x];
	Vertex v1 = vertices[nonuniformEXT(objID)].v[ind.y];
	Vertex v2 = vertices[nonuniformEXT(objID)].v[ind.z];

	// Get the interpolation coefficients
	const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

	// Interpolate the position and normal vector for this ray
	vec4 modelNormal = vec4(v0.nrm * barycentrics.x + v1.nrm * barycentrics.y + v2.nrm * barycentrics.z, 1.0);
	vec4 modelPos = vec4(v0.pos * barycentrics.x + v1.pos * barycentrics.y + v2.pos * barycentrics.z, 1.0);

	// Transform the position and normal vectors from model space to world space
	mat4 worldTransform = transforms[0].t[nonuniformEXT(objID)];
	vec3 worldPos = (worldTransform * modelPos).xyz;

	// Don't translate the normal vector, only rotate and scale
	mat3 worldRotate = mat3(worldTransform[0].xyz, worldTransform[1].xyz, worldTransform[2].xyz);
	vec3 worldNormal = worldRotate * modelNormal.xyz;

	// Get the material associated to the current object.
	vec3 diffuseColor = materials[0].m[objID].rgb;
	vec3 fullShadowColor = diffuseColor * 0.025;

	// Per light in the scene (Currently one)
	vec3 outColor = diffuseColor * lightColor.xyz;

	// Calculate how strong the shadowing is at this location.
	vec3 directionToLight = lightPosition.xyz - worldPos;
	vec3 normalizedDirectionToLight = normalize(directionToLight);
	vec3 normalizedWorldNormal = normalize(worldNormal);

	float dotProd = dot(normalizedDirectionToLight, normalizedWorldNormal);
	float dotVal = max(dotProd, 0.0);

	// If the fragment is considered totally in shadow there's no point tracing a ray for it
	if (dotProd > 0.0)
	{
		// Trace shadow rays
		uint rayFlags = gl_RayFlagsOpaqueEXT;
		float tMin = 0.01;
		float tMax = length(directionToLight);
		traceRayEXT(topLevelAS, // acceleration structure
			rayFlags, // rayFlags
			0xFF, // cullMask
			1, // sbtRecordOffset
			0, // sbtRecordStride
			1, // missIndex
			worldPos + 0.01 * worldNormal, // ray origin
			tMin, // ray min range
			directionToLight, // ray direction
			tMax, // ray max range
			1 // payload (location = 1)
		);
		dotVal *= shdPayload.miss;
	}

	outColor = dotVal * diffuseColor + (1 - dotVal) * fullShadowColor;
	payload.hitValue = outColor;
}
