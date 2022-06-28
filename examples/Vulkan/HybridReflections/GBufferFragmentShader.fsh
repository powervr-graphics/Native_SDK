#version 450

struct Material
{
	ivec4 textureIndices;
	vec4 baseColor;
	vec4 metallicRoughness;
};

layout(set = 0, binding = 0) uniform GlobalUBO
{
	highp mat4 mViewMatrix;
	highp mat4 mProjectionMatrix;
	highp mat4 mInvVPMatrix;
	highp vec4 vCameraPosition;
};

layout(set = 0, binding = 2) buffer MateralDataBufferBuffer { Material materials[]; } ;
layout(set = 0, binding = 4) uniform sampler2D textureSamplers[4];

layout(push_constant) uniform PushConsts {
	layout(offset = 4) uint materialID;
};

layout(location = 0) in mediump vec2 vTexCoord;
layout(location = 1) in highp vec3 vNormal;
layout(location = 2) in highp vec3 vWorldPosition;

layout(location = 0) out highp vec4 oAlbedo;
layout(location = 1) out highp vec4 oNormal_Roughness;
layout(location = 2) out highp vec4 oWorldPosition_Metallic;

void main()
{
	const Material mat = materials[materialID];

	vec3 albedo;
	float roughness = max(mat.metallicRoughness.g, 0.01);
	float metallic = mat.metallicRoughness.r;

	// If a valid texture index does not exist, use the albedo color stored in the Material structure
	if (mat.textureIndices.x == -1)
		albedo = mat.baseColor.rgb;
	else // If a valid texture index exists, use it to index into the image sampler array and sample the texture
		albedo = texture(textureSamplers[mat.textureIndices.x], vTexCoord).rgb;

	oAlbedo = vec4(albedo, 1.0f);
	oNormal_Roughness = vec4(normalize(vNormal), roughness);
	oWorldPosition_Metallic = vec4(vWorldPosition, metallic);
}
