#version 450

layout(set = 0, binding = 0) uniform StaticPerScene
{
	highp float farClipDistance;
};

layout(set = 1, binding = 1) uniform DynamicsPerPointLight
{
	highp mat4 mWorldViewProjectionMatrix;
	highp vec4 vViewPosition;
	highp mat4 mProxyWorldViewProjectionMatrix;
	highp mat4 mProxyWorldViewMatrix;
};

layout(set = 1, binding = 0) uniform StaticsPerPointLight
{
	highp float fLightIntensity;
	highp float fLightRadius;
	highp vec4 vLightColor;
	highp vec4 vLightSourceColor;
};

layout(input_attachment_index = 0, set = 2, binding = 0) uniform subpassInput localMemAlbedo;     //this is local memory
layout(input_attachment_index = 1, set = 2, binding = 1) uniform subpassInput localMemNormal;     //this is local memory
layout(input_attachment_index = 2, set = 2, binding = 2) uniform subpassInput localMemDepth;      //this is local memory

layout(location = 0) out lowp vec4 oColorFbo;

layout(location = 0) in highp vec3 vPositionVS;
layout(location = 1) in highp vec3 vViewDirVS;
layout(location = 2) in mediump vec2 vTexCoord;

void main()
{		
	//
	// Read GBuffer attributes
	//
	highp vec4 albedoSpec = subpassLoad(localMemAlbedo);
	highp vec3 normalTex =  subpassLoad(localMemNormal).xyz;
	
	// reconstruct original depth value
	highp float depth = subpassLoad(localMemDepth).x;

	//
	// Reconstruct view space position 
	//
	highp vec3 viewRay = vec3(vPositionVS.xy * (farClipDistance / vPositionVS.z), farClipDistance);
	highp vec3 positionVS = viewRay * depth;
	
	//
	// Calculate view space light direction
	//
	highp vec3 lightDirection = vViewPosition.xyz - positionVS;
	highp float lightDistance = length(lightDirection);
	lightDirection /= lightDistance;

	//
	// Calculate lighting terms
	//
	highp vec3 normal = normalize(normalTex * 2.0 - 1.0);	
	highp float n_dot_l = max(dot(lightDirection, normal), 0.0);	
	highp vec3 diffuse = n_dot_l * albedoSpec.rgb;

	highp vec3 viewDirection = normalize(vViewDirVS);
	highp vec3 reflectedLightDirection = reflect(lightDirection, normal);
	highp float v_dot_r = max(dot(viewDirection, reflectedLightDirection), 0.0);
	diffuse += vec3(pow(v_dot_r, 16.0) * albedoSpec.a);

	// calculate an attenuation factor
	highp float attenuation = clamp(1.0 - lightDistance / fLightRadius, 0.0, 1.0);
	attenuation *= attenuation;

	oColorFbo = vec4(fLightIntensity * diffuse * vLightColor.rgb * attenuation, 1.0);
}