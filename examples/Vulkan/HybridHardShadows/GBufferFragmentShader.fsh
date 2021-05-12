#version 450

struct Material
{
	ivec4 textureIndices;
	vec4 baseColor;
	vec4 metallicRoughnessReflectivity;
	vec4 f0f90;
};

layout(set = 0, binding = 0) uniform GlobalUBO
{
	highp mat4 mViewMatrix;
	highp mat4 mProjectionMatrix;
	highp vec4 vAmbientLightColor;
	highp vec4 vCameraPosition;
	uint uNumLights;
};

layout(set = 0, binding = 2) buffer MateralDataBufferBuffer { Material materials[]; } ;
layout(set = 0, binding = 3) uniform sampler2D textureSamplers[1];

layout(push_constant) uniform PushConsts {
	layout(offset = 64) uint materialID;
};

layout(location = 0) in mediump vec2 vTexCoord;
layout(location = 1) in mediump vec3 vNormal;
layout(location = 2) in highp vec3 vWorldPosition;

layout(location = 0) out highp vec4 oAlbedo_Metallic;
layout(location = 1) out highp vec4 oNormal_Reflectivity;
layout(location = 2) out highp vec4 oWorldPosition_F90;
layout(location = 3) out highp vec4 oF0_Rougness;

void main()
{
	const Material mat = materials[materialID];

	vec3 albedo;
	float roughness = mat.metallicRoughnessReflectivity.y;
	float metallic = mat.metallicRoughnessReflectivity.x;
	float reflectivity = mat.metallicRoughnessReflectivity.z;
	vec3 F0 = mat.f0f90.xyz;
	float F90 = mat.f0f90.w;

	// If a valid texture index does not exist, use the albedo color stored in the Material structure
	if (mat.textureIndices.x == -1)
		albedo = mat.baseColor.rgb;
	else // If a valid texture index exists, use it to index into the image sampler array and sample the texture
		albedo = texture(textureSamplers[mat.textureIndices.x], vTexCoord).rgb;

	oAlbedo_Metallic = vec4(albedo, metallic);
	oNormal_Reflectivity = vec4(normalize(vNormal), reflectivity);
	oWorldPosition_F90 = vec4(vWorldPosition, F90);
	oF0_Rougness = vec4(F0, roughness);
}
