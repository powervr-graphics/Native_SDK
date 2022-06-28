#version 460

#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable

struct Material
{
	mediump vec4 baseColor;              // Base color in case no texture is available to sample
	mediump int reflectanceTextureIndex; // Reflectance texture index
	mediump int isTransparent;           // Flag to know if the material is transparent or opaque
	mediump float transparency;          // 0.0 means totally transparent surface, 1.0 means totally opaque (but not light blocking) surface
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
layout(location = 1) in highp vec3 vNormal;
layout(location = 2) in highp vec3 vWorldPosition;

layout(location = 0) out highp vec4 outReflectance;
layout(location = 1) out highp vec4 outNormalMaterialID;
layout(location = 2) out highp vec4 outWorldPositionTransparency;

void main()
{
	const Material mat     = materials.m[materialID];
	highp vec3 reflectance = mat.baseColor.xyz; // If a valid texture index does not exist, use the albedo color stored in the Material structure

	if (mat.reflectanceTextureIndex != -1)
	{
		// If a valid texture index exists, use it to index into the image sampler array and sample the texture
		reflectance = texture(textureSamplers[mat.reflectanceTextureIndex], vTexCoord).rgb;
	}

	float transparencyFlag;

	if(mat.isTransparent == 1)
	{
		transparencyFlag = 2.0;
	}
	else
	{
		transparencyFlag = 4.0;
	}

	outReflectance               = vec4(reflectance, 1.0); // The value 1 in the w component is to distinguish in the final pass scene geometry from background
	outNormalMaterialID          = vec4(normalize(vNormal), float(materialID));
	outWorldPositionTransparency = vec4(vWorldPosition, 0.0);
}
