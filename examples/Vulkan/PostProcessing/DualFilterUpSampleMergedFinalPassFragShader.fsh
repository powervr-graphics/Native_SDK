#version 320 es

layout(constant_id = 0) const int RenderBloom = 0;

layout(set = 0, binding = 0) uniform mediump sampler2D sTexture;
layout(set = 0, binding = 1) uniform mediump sampler2D sOffScreenTexture;
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

	// Retrieve the original hdr color attachment and combine it with the blurred image
	mediump vec3 offscreenTexture = texture(sOffScreenTexture, vTexCoords[0]).rgb;

	mediump vec3 hdrColor;

	if(RenderBloom == 1)
	{
		hdrColor = vec3(blurredColor);
	}
	else
	{
		hdrColor = offscreenTexture + vec3(blurredColor);
	}

	// http://filmicworlds.com/blog/filmic-tonemapping-operators/
	// Reinhard tonemapping
	mediump vec3 ldrColor = hdrColor * ExposureBias;
	ldrColor = ldrColor / (1.0 + ldrColor);

	// apply a simple vignette
	mediump vec2 vtc = vec2(vTexCoords[0] - vec2(0.5));
	// determine the vector length of the center position
	mediump float lenPos = length(vtc);
	mediump float vignette = smoothstep(VignetteRadius, VignetteRadius - VignetteSoftness, lenPos);

	oColor = vec4(ldrColor * vignette, 1.0);
}