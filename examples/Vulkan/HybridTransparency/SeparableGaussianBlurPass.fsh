#version 320 es

layout(set = 0, binding = 0) uniform mediump sampler2D rayTracedTransparency;

layout(location = 0) in mediump vec2 inUV[2];
layout(location = 2) in mediump vec2 uvNoOffset;
layout(location = 4) in mediump float inWeight[2];

layout(location = 0) out mediump vec4 outFragColor;

void main()
{
	mediump vec4 color0 = texture(rayTracedTransparency, inUV[0]);
	mediump vec4 color1 = texture(rayTracedTransparency, inUV[1]);

	// The ray traced render target carries in the .w field values to compose the final scene that should not be mixed.

	if((color0.w == 0.4) || (color1.w == 0.4))
	{
		// At least one texel cooresponds to ray traced projected transparency, apply blur
		outFragColor.xyz = color0.xyz * inWeight[0] + color1.xyz * inWeight[1];
		outFragColor.w   = max(color0.w, color1.w); // Prefer directly lit areas (.w is 1.0) over areas with projected transparency (.w is 0.4)
	}
	else
	{
		// Do not blur
		outFragColor = texture(rayTracedTransparency, uvNoOffset);
	}
}
