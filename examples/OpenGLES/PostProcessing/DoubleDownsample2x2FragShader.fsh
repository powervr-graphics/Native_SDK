#version 310 es
#extension GL_IMG_framebuffer_downsample : enable

layout(binding = 0) uniform mediump sampler2D sTexture;

layout(location = 0) in mediump vec2 vTexCoord;

layout(location = 0) out mediump float oLuminance;

#ifdef GL_IMG_framebuffer_downsample
layout(location = 1) out mediump float oDownsampledFilter;
#endif

void main()
{
	oLuminance = texture(sTexture, vTexCoord).r;

#ifdef GL_IMG_framebuffer_downsample
	oDownsampledFilter = oLuminance;
#endif
}