#version 450

layout(input_attachment_index = 0, set = 0, binding = 2) uniform subpassInput localMemAlbedo;     //this is local memory
layout(input_attachment_index = 1, set = 0, binding = 3) uniform subpassInput localMemNormal;   //this is local memory
layout(input_attachment_index = 2, set = 0, binding = 4) uniform subpassInput localMemDepth;    //this is local memory

layout(location = 0) out lowp vec4 oColorFbo;

layout(set = 0, binding = 0) uniform StaticPerDirectionalLight
{
	highp vec4 fLightIntensity;
};

layout(set = 0, binding = 1) uniform DynamicPerDirectionalLight
{	
	highp vec4 vViewDirection;
};

void main()
{
	// Fetch required gbuffer attributes
	lowp  vec3 albedo = subpassLoad(localMemAlbedo).rgb;
	highp vec3 normalTex = subpassLoad(localMemNormal).rgb;
	highp vec3 normal = normalize(normalTex.xyz * 2.0 - 1.0);
	
	// Calculate simple diffuse lighting
	highp float n_dot_l = max(dot(-vViewDirection.xyz, normal.xyz), 0.0);
	lowp vec3 color = albedo * (n_dot_l * fLightIntensity.rgb + vec3(.2, .2, .1));
	
	oColorFbo = vec4(color, 1.); 
}