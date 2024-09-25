#version 320 es

layout(constant_id = 0) const int RenderBloom = 0;

layout(set = 0, binding = 0) uniform highp sampler2D sCurrentBlurredImage;
layout(set = 0, binding = 1) uniform highp sampler2D sDownsampledCurrentMipLevel;
layout(set = 0, binding = 2) uniform highp sampler2D sOffScreenTexture;
layout(location = 0) in highp vec2 vTexCoords[9];
layout(location = 0) out highp vec4 oColor;

const highp float weights[9] = float[9](0.25, 0.0625, 0.125, 0.0625, 0.125, 0.0625, 0.125, 0.0625, 0.125);

// Radius for our vignette
const highp float VignetteRadius = 0.85;

// Soft for our vignette, between 0.0 and 1.0
const highp float VignetteSoftness = 0.35;

// Scale value to prevent the luminance from overflowing
const highp float FP16Scale = 10.0;

layout(push_constant) uniform pushConstantsBlock{
	highp vec2 upSampleConfigs[8];
	highp float linearExposure;
};

void main()
{	
	highp float blurredColor = texture(sDownsampledCurrentMipLevel, vTexCoords[0]).r * weights[0];
	
	for(int i = 0; i < 9; ++i)
	{
		blurredColor += texture(sCurrentBlurredImage, vTexCoords[i]).r * weights[i];
	}
		
	// Multiply by the FP16 scale value to restore the original luminance
	blurredColor *= FP16Scale;

	highp vec3 hdrColor;

	if(RenderBloom == 1)
	{
		hdrColor = vec3(blurredColor);
	}
	else
	{
		// Retrieve the original hdr colour attachment and combine it with the blurred image
		highp vec3 offscreenTexture = texture(sOffScreenTexture, vTexCoords[0]).rgb;

		hdrColor = offscreenTexture * linearExposure + vec3(blurredColor);
	}

	// http://filmicworlds.com/blog/filmic-tonemapping-operators/
	// Reinhard tone mapping
	highp vec3 ldrColor = hdrColor / (1.0 + hdrColor);

	// apply a simple vignette
	highp vec2 vtc = vec2(vTexCoords[0] - vec2(0.5));
	// determine the vector length of the centre position
	highp float lenPos = length(vtc);
	highp float vignette = smoothstep(VignetteRadius, VignetteRadius - VignetteSoftness, lenPos);

	oColor = vec4(ldrColor * vignette, 1.0);
}