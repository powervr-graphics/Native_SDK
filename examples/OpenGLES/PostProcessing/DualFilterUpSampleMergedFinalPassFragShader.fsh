#version 310 es

layout(binding = 0) uniform mediump sampler2D sTexture;
layout(binding = 1) uniform mediump sampler2D sOffScreenTexture;
layout(location = 0) in mediump vec2 vTexCoords[9];

layout(location = 0) out mediump vec4 oColor;

// Radius for our vignette
const mediump float VignetteRadius = 0.85;

// Soft for our vignette, between 0.0 and 1.0
const mediump float VignetteSoftness = 0.35;

const mediump float ExposureBias = 4.0;

void main()
{
	mediump float blurredColor = texture(sTexture, vTexCoords[1]).r;
	blurredColor += texture(sTexture, vTexCoords[2]).r * 2.0;
	blurredColor += texture(sTexture, vTexCoords[3]).r;
	blurredColor += texture(sTexture, vTexCoords[4]).r * 2.0;
	blurredColor += texture(sTexture, vTexCoords[5]).r;
	blurredColor += texture(sTexture, vTexCoords[6]).r * 2.0;
	blurredColor += texture(sTexture, vTexCoords[7]).r;
	blurredColor += texture(sTexture, vTexCoords[8]).r * 2.0;
	blurredColor *= 0.08333333333333333333333333333333;

	mediump vec3 hdrColor;

#ifdef RENDER_BLOOM
	hdrColor = vec3(blurredColor);
#else
	mediump vec3 offscreenTexture = texture(sOffScreenTexture, vTexCoords[0]).rgb;

	hdrColor = offscreenTexture + vec3(blurredColor);
#endif

	// http://filmicworlds.com/blog/filmic-tonemapping-operators/
	// Reinhard tonemapping
	mediump vec3 ldrColor = hdrColor * ExposureBias;
	ldrColor = ldrColor / (1.0 + ldrColor);

	// apply a simple vignette
	mediump vec2 vtc = vec2(vTexCoords[0] - vec2(0.5));
	// determine the vector length of the center position
	mediump float lenPos = length(vtc);
	mediump float vignette = smoothstep(VignetteRadius, VignetteRadius - VignetteSoftness, lenPos);

	mediump vec3 color = ldrColor * vignette;
#ifndef FRAMEBUFFER_SRGB
	color = pow(color, vec3(0.4545454545)); // Do gamma correction
#endif	

	oColor = vec4(color, 1.0);
}