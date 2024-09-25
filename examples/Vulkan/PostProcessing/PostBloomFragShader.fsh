#version 320 es

layout(constant_id = 0) const int RenderBloom = 0;

layout(set = 0, binding = 0) uniform highp sampler2D sBlurTexture;
layout(set = 0, binding = 1) uniform highp sampler2D sOffScreenTexture;

layout(location = 0) in highp vec2 vTexCoords;

layout(location = 0) out highp vec4 oColor;

// Radius for our vignette
const highp float VignetteRadius = 0.85;

// Soft for our vignette, between 0.0 and 1.0
const highp float VignetteSoftness = 0.35;

// Scale value to prevent the luminance from overflowing
const highp float FP16Scale = 10.0;

layout(push_constant) uniform pushConstantsBlock{
	highp float linearExposure;
};

void main()
{
	highp vec3 hdrColor;

	highp vec3 bloomTexture = vec3(texture(sBlurTexture, vTexCoords).r);

	// Multiply by the FP16 scale value to restore the original luminance
	bloomTexture *= FP16Scale;

	if(RenderBloom == 1)
	{
		hdrColor = bloomTexture;
	}
	else
	{
		// Retrieve the two hdr colour attachments and combine them
		highp vec3 offscreenTexture = texture(sOffScreenTexture, vTexCoords).rgb;
		hdrColor = offscreenTexture * linearExposure + bloomTexture;
	}

	// http://filmicworlds.com/blog/filmic-tonemapping-operators/
	// Reinhard tone mapping
	highp vec3 ldrColor = hdrColor / (1.0 + hdrColor);

	// apply a simple vignette
	highp vec2 vtc = vec2(vTexCoords - vec2(0.5));
	// determine the vector length of the centre position
	highp float lenPos = length(vtc);
	highp float vignette = smoothstep(VignetteRadius, VignetteRadius - VignetteSoftness, lenPos);

	oColor = vec4(ldrColor.x * vignette, ldrColor.y * vignette, ldrColor.z * vignette, 1.0);
}
