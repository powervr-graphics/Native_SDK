#version 440
layout (set = 0, binding = 0)uniform sampler2D  sBaseTex;
layout (set = 0, binding = 1)uniform sampler2D  sNormalMap;
		
layout (location = 0)in lowp    vec3  LightVec;
layout (location = 1)in mediump vec2  TexCoord;
layout (location = 0) out lowp vec4 oColor;

void main()
{
	// read the per-pixel normal from the normal map and expand to [-1, 1]
	lowp vec3 normal = texture(sNormalMap, TexCoord).rgb * 2.0 - 1.0;

    // linear interpolations of normals may cause shortened normals and thus
	// visible artifacts on low-poly models.
	// We omit the normalization here for performance reasons

    
	// calculate diffuse lighting as the cosine of the angle between light
	// direction and surface normal (both in surface local/tangent space)
	// We don't have to clamp to 0 here because the framebuffer write will be clamped
	lowp float lightIntensity = dot(LightVec, normal) + .2/*ambient*/;

	// read base texture and modulate with light intensity
	lowp vec3 texColor = texture(sBaseTex, TexCoord).rgb;	
	oColor = vec4(texColor * lightIntensity, 1.0);
}
