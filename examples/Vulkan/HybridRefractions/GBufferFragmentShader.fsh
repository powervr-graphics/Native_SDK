#version 460

#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable

struct Material
{
	mediump vec4 baseColor;
	mediump int reflectanceTextureIndex;
	mediump float indexOfRefraction;
	mediump float attenuationCoefficient;
};

layout(set = 0, binding = 0) uniform GlobalUBO
{
	highp mat4 viewMatrix;
	highp mat4 projectionMatrix;
	highp mat4 inverseViewProjectionMatrix;
	highp vec4 cameraPosition;
};

layout(set = 0, binding = 2, scalar) buffer MateralDataBufferBuffer { Material m[]; } materials;
layout(set = 0, binding = 4) uniform mediump sampler2D textureSamplers[];

layout(push_constant) uniform PushConsts { uint materialID; };

layout(location = 0) in mediump vec2 vTexCoord;
layout(location = 1) in mediump vec3 vNormal;
layout(location = 2) in highp vec3 vWorldPosition;

layout(location = 0) out highp vec4 outReflectance;
layout(location = 1) out highp vec4 outNormalMaterialID;
layout(location = 2) out highp vec4 outWorldPositionIOR;

void main()
{
	const Material mat = materials.m[materialID];
	highp vec3 reflectance   = mat.baseColor.xyz; // If a valid texture index does not exist, use the albedo color stored in the Material structure

	if (mat.reflectanceTextureIndex != -1)
	{
		// If a valid texture index exists, use it to index into the image sampler array and sample the texture
		reflectance = texture(textureSamplers[mat.reflectanceTextureIndex], vTexCoord).rgb;
	}

	outReflectance      = vec4(reflectance, 1.0);
	outNormalMaterialID = vec4(normalize(vNormal), float(materialID));
	outWorldPositionIOR = vec4(vWorldPosition, mat.indexOfRefraction);
}
