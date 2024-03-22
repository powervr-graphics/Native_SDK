#version 320 es

layout (set = 0, binding = 0) uniform mediump sampler2D sBaseTex;
layout (set = 0, binding = 1) uniform mediump sampler2D sNormalMap;
 		
layout(location = 0) in mediump vec3 LightVec;
layout(location = 1) in mediump vec2 TexCoord;
layout(location = 2) in highp vec4 currentFramePosition;
layout(location = 3) in highp vec4 prevFramePosition;  

layout(location = 0) out mediump vec4 FragColor;
layout(location = 1) out highp vec4 velColor;

void main()
{
	// read the per-pixel normal from the normal map and expand to [-1, 1]
	mediump vec3 normal = texture(sNormalMap, TexCoord).rgb * 2.0 - 1.0;

    // linear interpolations of normals may cause shortened normals and thus
	// visible artifacts on low-poly models.
	// We omit the normalization here for performance reasons

	// calculate diffuse lighting as the cosine of the angle between light
	// direction and surface normal (both in surface local/tangent space)
	// We don't have to clamp to 0 here because the framebuffer write will be clamped
	mediump float lightIntensity = dot(LightVec, normal) + .2 /*ambient*/;

	// read base texture and modulate with light intensity
	mediump vec3 texColor = texture(sBaseTex, TexCoord).rgb;	
	FragColor = vec4(texColor * lightIntensity, 1.0);

	// calculate velocity
	highp vec4 currPos = currentFramePosition;
	highp vec4 prePos = prevFramePosition;

	currPos = currPos / currPos.w;
	currPos.xy = (currPos.xy + 1.0f) / 2.0f;
	currPos.y = 1.0f - currPos.y;

	prePos = prePos / prePos.w;
	prePos.xy = (prePos.xy + 1.0f) / 2.0f;
	prePos.y = 1.0f - prePos.y;

	highp vec2 velocity = (currPos - prePos).xy;
	velColor = vec4(abs(velocity), 0.0f, 1.0f);
}