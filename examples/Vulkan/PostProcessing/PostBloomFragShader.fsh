#version 320 es

layout(constant_id = 0) const int RenderBloom = 0;
layout(constant_id = 1) const int RenderOffScreenOnly = 0;

layout(set = 0, binding = 0) uniform mediump sampler2D sBlurTexture;
layout(set = 0, binding = 1) uniform mediump sampler2D sOffScreenTexture;

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

	if(RenderOffScreenOnly == 1)
	{
		hdrColor = texture(sOffScreenTexture, vTexCoords).rgb;
	}
	else if(RenderBloom == 1)
	{
		hdrColor = vec3(texture(sBlurTexture, vTexCoords).r);
	}
	else
	{
		// Retrieve the two hdr color attachments and combine them
		mediump vec3 offscreenTexture = texture(sOffScreenTexture, vTexCoords).rgb;
		mediump vec3 bloomTexture = vec3(texture(sBlurTexture, vTexCoords).r);
		hdrColor = offscreenTexture + bloomTexture;
	}

	// http://filmicworlds.com/blog/filmic-tonemapping-operators/
	// Reinhard tonemapping
	mediump vec3 ldrColor = hdrColor * ExposureBias;
	ldrColor = ldrColor / (1.0 + ldrColor);

	// apply a simple vignette
	mediump vec2 vtc = vec2(vTexCoords - vec2(0.5));
	// determine the vector length of the center position
	mediump float lenPos = length(vtc);
	mediump float vignette = smoothstep(VignetteRadius, VignetteRadius - VignetteSoftness, lenPos);
	if(RenderOffScreenOnly == 1)
	{
		vignette = 1.0;
	}

	oColor = vec4(ldrColor * vignette, 1.0);
}
