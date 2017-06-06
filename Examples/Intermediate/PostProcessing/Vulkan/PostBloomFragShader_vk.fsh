#version 450
layout (set = 0, binding = 0) uniform sampler2D sTexture;
layout (set = 0, binding = 1) uniform sampler2D sBlurTexture;

layout (set = 1, binding = 0) uniform BloomConfig
{
	mediump float sTexFactor;
	mediump float sBlurTexFactor;
};

in mediump vec2 vTexCoord;
layout (location = 0) out mediump vec4 oColor;

void main()
{
	oColor = (texture(sTexture, vTexCoord) * sTexFactor) + (texture(sBlurTexture, vTexCoord) * sBlurTexFactor);
}
