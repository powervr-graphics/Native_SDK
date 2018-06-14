#version 310 es
#extension GL_EXT_shader_pixel_local_storage2 : enable

#ifndef GL_EXT_shader_pixel_local_storage2
#extension GL_EXT_shader_pixel_local_storage : require
#endif

layout(rgba8)  __pixel_localEXT FragDataLocal {
	layout(rgba8) highp vec4 albedo;
	layout(rgb10_a2) highp vec4 normal;
	layout(r32f) highp float depth;
	layout(rgba8) highp vec4 color;
} pls;

layout(binding = 0) uniform StaticPerDirectionalLight
{
	highp vec4 vLightIntensity;
	highp vec4 vAmbientLight;
};

layout(binding = 1) uniform DynamicPerDirectionalLight
{	
	highp vec4 vViewSpaceLightDirection;
};

void main()
{
	// Fetch required gbuffer attributes
	highp vec3 normalTex = pls.normal.rgb;
	highp vec3 normal = normalize(normalTex.xyz * 2.0 - 1.0);
	
	// Calculate simple diffuse lighting
	highp float n_dot_l = max(dot(-vViewSpaceLightDirection.xyz, normal.xyz), 0.0);
	highp vec3 lightColor = (n_dot_l * vLightIntensity.rgb + vAmbientLight.rgb);
	lowp vec3 color = pls.albedo.rgb * lightColor;
	pls.color = vec4(color, 1.);
}
