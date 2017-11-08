#version 310 es
#extension GL_EXT_shader_pixel_local_storage2 : enable

#ifndef GL_EXT_shader_pixel_local_storage2
#extension GL_EXT_shader_pixel_local_storage : require
#endif

uniform highp float fFarClipDistance;

layout(std140, binding = 0) uniform DynamicsPerPointLight
{
	highp mat4 mWorldViewProjectionMatrix;
	highp vec4 vViewPosition;
	highp mat4 mProxyWorldViewProjectionMatrix;
	highp mat4 mProxyWorldViewMatrix;
};

layout(std140, binding = 1) uniform StaticsPerPointLight
{
	highp float fLightIntensity;
	highp float fLightRadius;
	highp vec4 vLightColor;
	highp vec4 vLightSourceColor;
};

layout(rgba8)  __pixel_localEXT FragDataLocal {
	layout(rgba8) highp vec4 albedo;
	layout(rgb10_a2) highp vec4 normal; 
	layout(r32f) highp float depth;
	layout(rgba8) highp vec4 color;
} pls;

layout(location = 0) in highp vec3 vPositionVS;
layout(location = 1) in highp vec3 vViewDirVS;

void main()
{
	// Fetch required gbuffer attributes
	highp vec3 normalTex = pls.normal.rgb;
	highp vec4 albedoSpec = pls.albedo;
	highp vec3 normal = normalize(normalTex.xyz * 2.0 - 1.0);

	//
	// Reconstruct view space position 
	//
	highp vec3 viewRay = vec3(vPositionVS.xy * (fFarClipDistance / vPositionVS.z), fFarClipDistance);
	highp vec3 positionVS = viewRay * pls.depth;
		
	//
	// Calculate view space light direction
	//
	highp vec3 lightDirection = vViewPosition.xyz - positionVS;
	highp float lightDistance = length(lightDirection);
	lightDirection /= lightDistance;

	//
	// Calculate lighting terms
	//
	highp float n_dot_l = max(dot(lightDirection, normal), 0.0);
	highp vec3 diffuse = n_dot_l * albedoSpec.rgb;

	highp vec3 viewDirection = normalize(vViewDirVS);
	highp vec3 reflectedLightDirection = reflect(lightDirection, normal);
	highp float v_dot_r = max(dot(viewDirection, reflectedLightDirection), 0.0);
	diffuse += vec3(pow(v_dot_r, 16.0) * albedoSpec.a);

	// calculate an attenuation factor
	highp float attenuation = clamp(1.0 - lightDistance / fLightRadius, 0.0, 1.0);
	attenuation *= attenuation;

	highp vec4 lightColor = vec4(fLightIntensity * diffuse * vLightColor.rgb * attenuation, 1.0);
	
	pls.color = pls.color + lightColor;
}