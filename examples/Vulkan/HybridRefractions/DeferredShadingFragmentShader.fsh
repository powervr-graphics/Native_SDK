#version 320 es

struct Material
{
	mediump vec4 baseColor;
	mediump int reflectanceTextureIndex;
	mediump float indexOfRefraction;
	mediump float attenuationCoefficient;
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
layout(set = 1, binding = 2) uniform mediump sampler2D gBufferWorldPositionIOR;
layout(set = 1, binding = 3) uniform mediump samplerCube skyboxImage;

layout(set = 2, binding = 0) uniform sampler2D refractionsImage;

layout(location = 0) in mediump vec2 inUV;
layout(location = 1) in highp vec3 inRayDir;

layout(location = 0) out mediump vec4 outFragColor;

highp vec3 evaluateDiffuseMaterial(mediump vec3 origin, mediump vec3 normal, highp vec3 lightPosition, highp vec3 lightcolor)
{
	highp vec3 originToLightPosition = lightPosition - origin;
	mediump float distSquared        = dot(originToLightPosition, originToLightPosition);
	originToLightPosition            = normalize(originToLightPosition);
	highp float thetaLight           = clamp(0.0, 1.0, dot(normal, originToLightPosition));
	highp vec3 Li                    = (lightData.ambientColorIntensity.w * lightcolor * thetaLight) / distSquared;
	return Li;
}

void main()
{
	// Unpack the values stored in the G-Buffer
	mediump vec4 reflectanceAndForegroundFlag = texture(gBufferReflectance, inUV);

	highp vec3 Li = vec3(0.0);

	// If the foregroundFlag is 0.0f, the fragment belongs to the skybox so we can sample the cubemap
	if (reflectanceAndForegroundFlag.w != 0.0)
	{
		mediump vec4 normalAndMaterialID     = texture(gBufferNormalMaterialID, inUV);
		mediump vec4 worldPositionAndIOR     = texture(gBufferWorldPositionIOR, inUV);
		mediump vec4 refractionsAndShadowRay = texture(refractionsImage, inUV);

		if (normalAndMaterialID.w < 2.0)
		{
			// Refractive material
			Li = refractionsAndShadowRay.xyz;
		}
		else
		{
			// Diffuse material
			Li = reflectanceAndForegroundFlag.xyz *
					evaluateDiffuseMaterial(worldPositionAndIOR.xyz, normalAndMaterialID.xyz, lightData.lightPosition.xyz, lightData.lightColor.xyz) * refractionsAndShadowRay.w +
				lightData.ambientColorIntensity.xyz * reflectanceAndForegroundFlag.xyz;
		}
	}
	else
	{
		highp vec3 direction = normalize(inRayDir);
		Li                   = texture(skyboxImage, direction).rgb;
	}

	outFragColor = vec4(Li, 1.0);
}
