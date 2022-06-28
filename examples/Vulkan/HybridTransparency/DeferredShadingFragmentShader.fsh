#version 320 es

struct Material
{
	mediump vec4 baseColor;              // Base color in case no texture is available to sample
	mediump int reflectanceTextureIndex; // Reflectance texture index
	mediump int isTransparent;           // Flag to know if the material is transparent or opaque
	mediump float transparency;          // 0.0 means totally transparent surface, 1.0 means totally opaque (but not light blocking) surface
};

struct LightData
{
	highp vec4 lightColor;
	highp vec4 lightPosition;
	highp vec4 ambientColorIntensity;
};

layout(set = 0, binding = 0) uniform GlobalUBO
{
	highp mat4 viewMatrix;
	highp mat4 projectionMatrix;
	highp mat4 inverseViewProjectionMatrix;
	highp vec4 cameraPosition;
};

layout(set = 0, binding = 1) uniform LightDataUBO { LightData lightData; };

layout(set = 1, binding = 0) uniform mediump sampler2D gBufferReflectance;
layout(set = 1, binding = 1) uniform mediump sampler2D gBufferNormalMaterialID;
layout(set = 1, binding = 2) uniform mediump sampler2D gBufferWorldPositionTransparency;
layout(set = 1, binding = 3) uniform mediump samplerCube skyboxImage;

layout(set = 2, binding = 0) uniform sampler2D transparencyImage;

layout(location = 0) in mediump vec2 inUV;
layout(location = 1) in highp vec3 inRayDir;

layout(location = 0) out mediump vec4 outFragColor;

highp vec3 evaluateDiffuseMaterial(highp vec3 worldPosition, highp vec3 normal, highp vec3 reflectance, highp vec3 lightColor)
{
	highp vec3 originToLightPosition = lightData.lightPosition.xyz - worldPosition;
	highp float distSquared          = dot(originToLightPosition, originToLightPosition);
	originToLightPosition            = normalize(originToLightPosition);
	highp float thetaLight           = clamp(0.0, 1.0, dot(normal, originToLightPosition));
	highp vec3 Li                    = ((lightData.ambientColorIntensity.w * lightColor * thetaLight * reflectance) / distSquared);

	return Li;
}

void main()
{
	// Unpack the values stored in the G-Buffer
	mediump vec4 reflectanceAndForegroundFlag = texture(gBufferReflectance, inUV);

	highp vec3 Li = vec3(0.0);

	// If the foregroundFlag is 0.0f, the fragment belongs to the skybox so we can sample the cubemap
	if (reflectanceAndForegroundFlag.w == 0.0)
	{
		highp vec3 direction = normalize(inRayDir);
		Li                   = texture(skyboxImage, direction).rgb;
	}
	else
	{
		mediump vec4 transparencyAndShadowRay = texture(transparencyImage, inUV);

		if((int(transparencyAndShadowRay.w * 10.0) == 2) || (int(transparencyAndShadowRay.w * 10.0) == 4))
		{
			// Use a step function to add ambient contribution to the projected transparency only (the flag value to identify see-through transparency is 0.2, for the projected transparency is .w = 0.4)
			Li = step(0.3, transparencyAndShadowRay.w) * lightData.ambientColorIntensity.xyz * reflectanceAndForegroundFlag.xyz + transparencyAndShadowRay.xyz;
		}
		else
		{
			mediump vec4 normalAndMaterialID          = texture(gBufferNormalMaterialID, inUV);
			mediump vec4 worldPositionAndTransparency = texture(gBufferWorldPositionTransparency, inUV);
			
			// The .w field is used to avoid direct lighting for those fragments in the GBuffer which are blocked when a ray towards the emitter was casted
			Li = transparencyAndShadowRay.w * evaluateDiffuseMaterial(worldPositionAndTransparency.xyz, normalAndMaterialID.xyz, reflectanceAndForegroundFlag.xyz, lightData.lightColor.xyz) + lightData.ambientColorIntensity.xyz * reflectanceAndForegroundFlag.xyz;
		}
	}

	outFragColor = vec4(Li, 1.0);
}
