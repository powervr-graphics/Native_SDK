#version 320 es

layout (set = 0, binding = 0) uniform mediump sampler2D Texture;
		
layout(location = 0) in highp vec2 TexCoords;

layout(location = 0) out mediump vec4 oColor;

void main()
{
	highp vec2 sampledTextureSize = vec2(textureSize(Texture, 0));
	oColor = texelFetch(Texture, ivec2(TexCoords * sampledTextureSize), 0);
	oColor.xyz = pow(oColor.xyz, vec3(0.454545));
}
