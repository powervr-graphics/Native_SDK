#version 310 es
#extension GL_EXT_shader_pixel_local_storage2 : enable

#ifndef GL_EXT_shader_pixel_local_storage2
#extension GL_EXT_shader_pixel_local_storage : require
#endif

uniform sampler2D sTexture;
uniform sampler2D sBumpMap;

uniform highp float fFarClipDistance;

layout(std140, binding = 1) uniform StaticPerMaterial
{
	highp float fSpecularStrength;
	highp vec4 vDiffuseColor;
};

layout (location = 0) in mediump vec2 vTexCoord;
layout (location = 1) in highp vec3 vNormal;
layout (location = 2) in highp vec3 vTangent;
layout (location = 3) in highp vec3 vBinormal;
layout (location = 4) in highp vec3 vViewPosition;

layout(rgba8)  __pixel_localEXT FragDataLocal {
	layout(rgba8) highp vec4 albedo;
	layout(rgb10_a2) highp vec4 normal;
	layout(r32f) highp float depth;
	layout(rgba8) highp vec4 color;
} pls;

void main()
{
	// Calculate the albedo
	// Pack the specular exponent with the albedo
	highp vec4 albedo = vec4(texture(sTexture, vTexCoord).rgb * vDiffuseColor.rgb, fSpecularStrength);
	
	// Calculate viewspace perturbed normal
	highp vec3 bumpmap = normalize(texture(sBumpMap, vTexCoord).rgb * 2.0 - 1.0);
	highp mat3 tangentSpace = mat3(normalize(vTangent), normalize(vBinormal), normalize(vNormal));	
	highp vec3 normalVS = tangentSpace * bumpmap;		

	// Scale the normal range from [-1,1] to [0, 1] to pack it into the RGB_U8 texture
	highp vec4 normal = vec4(normalVS * 0.5 + 0.5, 1.0);
	
	highp float depth = vViewPosition.z / fFarClipDistance;
		
	pls.albedo = albedo;
	pls.normal = normal;
	pls.depth = depth;

#ifndef GL_EXT_shader_pixel_local_storage2
	// clear pixel local storage color when GL_EXT_shader_pixel_local_storage2 isn't supported
	pls.color = vec4(0.0);
#endif
}