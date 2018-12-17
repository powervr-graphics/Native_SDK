#version 310 es

layout(binding = 0) uniform mediump sampler2D sBlurTexture;
layout(binding = 1) uniform mediump sampler2D sOffScreenTexture;

layout(location = 0) in mediump vec2 vTexCoords;

layout(location = 0) out mediump vec4 oColor;

// Radius for our vignette
const mediump float VignetteRadius = 0.85;

// Soft for our vignette, between 0.0 and 1.0
const mediump float VignetteSoftness = 0.35;

const mediump float ExposureBias = 4.0;

void main()
{
	mediump vec3 hdrColor;

#ifdef RENDER_OFFSCREEN
	hdrColor = texture(sOffScreenTexture, vTexCoords).rgb;
#elif defined(RENDER_BLOOM)
	hdrColor = vec3(texture(sBlurTexture, vTexCoords).r);
#else
	// Retrieve the two hdr color attachments and combine them
	mediump vec3 offscreenTexture = texture(sOffScreenTexture, vTexCoords).rgb;
	mediump vec3 bloomTexture = vec3(texture(sBlurTexture, vTexCoords).r);
	hdrColor = offscreenTexture + bloomTexture;
#endif

	// http://filmicworlds.com/blog/filmic-tonemapping-operators/
	// Reinhard tonemapping
	mediump vec3 ldrColor = hdrColor * ExposureBias;
	ldrColor = ldrColor / (1.0 + ldrColor);

	// apply a simple vignette
	mediump vec2 vtc = vec2(vTexCoords - vec2(0.5));
	// determine the vector length of the center position
	mediump float lenPos = length(vtc);
	mediump float vignette = smoothstep(VignetteRadius, VignetteRadius - VignetteSoftness, lenPos);
#ifdef RENDER_OFFSCREEN
	vignette = 1.0;
#endif

	mediump vec3 color = ldrColor * vignette;
#ifndef FRAMEBUFFER_SRGB
	color = pow(color, vec3(0.4545454545)); // Do gamma correction
#endif
	oColor = vec4(color, 1.0);
}
